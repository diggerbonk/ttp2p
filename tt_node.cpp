//
// File     : $Id$
// Author   : Trent McNair
//
// TTNode implements the basic functionality of a node on the TTP2P network.

#include <iostream>
#include <cstring>
#include <stdlib.h>

#include "ttools/tt_hashtable.h"
#include "ttools/tt_linked_list.h"
#include "ttools/tt_mutex.h"
#include "ttools/tt_functions.h"

#include "tt_packet.h"
#include "tt_packet_cache.h"
#include "tt_packet_network.h"
#include "tt_node_info.h"
#include "tt_node.h"
#include "tt_handler.h"
#include "tt_node_table.h"
#include "tt_connections.h"

TTNode::TTNode() : TTNotify()
{
   node_table = new TTNodeTable(4007);
   packet_cache = new TTPacketCache();
   packet_handlers = new TTHashtable(127);
   packet_listeners = new TTLinkedList();
   connections = new TTConnections();
   this_node = NULL;
   current_packet_id = 0;
   status = TT_NODE_STATUS_OFF;
   mutex = new TTMutex();
   network = new TTPacketNetwork(this);
   listen_ip = NULL;
   listen_port = TT_DEFAULT_PORT;
   
   min_connections = TT_DEFAULT_MIN_CONNECTIONS;
   max_connections = TT_DEFAULT_MAX_CONNECTIONS;
}

TTNode::~TTNode()
{
   // TODO: Stop everything, delete it all.
   delete [] listen_ip;
   delete this_node;
   delete mutex;
   
   // node_table
   // packet_cache
   // packet_handlers
   // packet_listeners
   // connections
}

//
// Start simply puts the node into READY mode and sets 
// the local ip address, port, and the alias.   If we're 
// already in READY mode, return false.

bool TTNode::Start(char * pIp, int pListenPort, char * pAlias)
{
   // we're doing a test/set here, pull out the mutex.
   mutex->Lock();   
   if ( status != TT_NODE_STATUS_OFF ) {
      mutex->Unlock();
      return false;
   }
   status = TT_NODE_STATUS_ON;
   mutex->Unlock();

   delete [] listen_ip;
   listen_ip = new char[ strlen(pIp) + 1];
   strcpy(listen_ip, pIp);
   listen_port = pListenPort;
   
   // set up the local node info.
   delete this_node;
   this_node = new TTNodeInfo(pIp, pListenPort);
   if ( pAlias ) {
      this_node->SetAlias(pAlias);
   }
   else this_node->SetAlias(this_node->id);
   
   TT_Debug("TTNode::Start %s", this_node->id);
   
   // Start the network listener
   network->Listen(listen_ip, listen_port);
   
   // If there is a node in the list, we'll try and connect to it.
   ManageConnections(); 
   
   return true;
}

void TTNode::DoNotify(long int networkId, int type, void * pData)
{
   TT_Debug("TTNode::DoNotify(%li, %i, data)", networkId, type);

   if ( type == TT_NOTIFY_IN ) {
      if ( pData ) {
         TTPacket * pkt = (TTPacket*)pData;
         pkt->hops++;
         switch ( pkt->route ) {
            case (TTPC_DISCOVER) :
               HandleDiscover(networkId, pkt);
               break;
            case (TTPC_BROADCAST) :
               HandleBroadcast(networkId, pkt);
               break;
            case (TTPC_INTERNAL) :
               HandleInternal(networkId, pkt);
               break;
            case (TTPC_PEER) :
               HandlePeer(networkId, pkt);
               break;
            case (TTPC_LOCAL) :
               HandleLocal(networkId, pkt);
               break;
         };    
      }
      else {
         TT_Debug("TTNode::DoNotify - NO PDATA??");
      }
   }
   else if ( type == TT_NOTIFY_END ) {
      // a network connection is down, we need to cleanup anything 
      // associated with that socket id.  This is a spendy 
      // operation, fortunately we do it in the last gasps of the 
      // network connection's thread.
      
      // 1. cleanup connections
      TT_Debug("TTNode::DoNotify - connection ended");
      connections->Remove(networkId);
      
      node_table->ClearNetworkID(networkId);
   }
}

void TTNode::HandleInternal(long int networkId, TTPacket * pkt)
{
   // This function might not be needed.
}

//
// Handle local packets.  Local packets are packets sent by nodes 
// directly connected to us and need no further routing.

void TTNode::HandleLocal(long int networkId, TTPacket * pkt)
{   
   // FILTER packets from ourselves
   char * packetType = (char*)pkt->GetString("type");
   if ( !packetType ) {
      TT_Error("TTNode::HandleLocal - packet missing type field\n");
      // TODO: disconnect ?
   }
   else if ( strcmp(packetType, "login") == 0 ) HandleLogin(networkId, pkt);
   else if ( strcmp(packetType, "welcome") == 0 ) HandleWelcome(networkId, pkt);
   else if ( strcmp(packetType, "connection_refused") == 0 ) HandleRefused(networkId, pkt);
   else {
      TT_Error("TTNode::HandleLocal - unknown packet type %s\n", packetType);
      // TODO: disconnect ?
   }
}

//
// Handle peer packets.

void TTNode::HandlePeer(long int networkId, TTPacket * pkt)
{   
   // FILTER packets from ourselves
   char * fromId = (char*)pkt->GetString("from");
   if ( !fromId ) {
      // malformed packet, no from field.
      TT_Error("TTNode::HandlePeer - no from field.");
      return;
   }
   else if ( strcmp(fromId, this_node->id) == 0 ) {
      // packet is from ourselves
      TT_Error("TTNode::HandlePeer - dropping self-routed packet");
      return;
   }
   
   if ( strcmp(fromId, this_node->id) == 0 ) {
      // TODO: packet is to us, handle it.
   }
   else {
      // the packet is not intented for us.
      TTNodeInfo * ttni = (TTNodeInfo*)node_table->Get(fromId);
      if ( ttni && (ttni->network_id > 0) ) {
         // we have a route to the node, send it there
         network->SendPacket(ttni->network_id, pkt);
      }
      else {
         // no route.  convert the packet to a discovery packet, 
         // cache it, 
         // packet, cache it, and broadcast it.
         pkt->route = TTPC_DISCOVER;
         char * charptr = (char*)pkt->GetString("id");
         if ( !charptr ) return;  // malformed packet.
         int messageId = atoi(charptr);
         packet_cache->Add(fromId, messageId);
         DoBroadcast(0, pkt);
      }
   }
}

void TTNode::HandleDiscover(long int networkId, TTPacket * pkt)
{
   // 1. FILTER packets from ourselves
   
   char * fromId = (char*)pkt->GetString("from");
   if ( !fromId ) {
      // malformed packet, no from field.
      TT_Error("TTNode::HandleBroadcast - no from field.");
      return;
   }
   else if ( strcmp(fromId, this_node->id) == 0 ) {
      // packet is from ourselves
      TT_Error("TTNode::HandleBroadcast - dropping self-routed packet");
      return;
   }
   
   // 2. FILTER DUPLICATE PACKETS / CACHE
   char * messageId = (char*)pkt->GetString("id");
   if ( !messageId ) return;  // malformed packet.
   int mid = atoi(messageId);
   if ( !packet_cache->Add(fromId, mid) ) {
      TT_Debug("TTNode::HandleDiscover - packet %s:%i in cache", fromId, mid);
      return;
   }

   if ( strcmp(fromId, this_node->id) == 0 ) {
      // if the packet is to us, handle 
      
      TTPacket * newPacket = new TTPacket();
      newPacket->route = TTPC_BROADCAST;
      newPacket->AddString("type", "trace");
      newPacket->AddLong("id", current_packet_id++);
      newPacket->AddString("from", this_node->id);
      network->SendPacket(networkId, newPacket);
      delete newPacket;
   }
   else {
      // if the packet is not to us, broadcast it.
      DoBroadcast(networkId, pkt);
   }
}

void TTNode::HandleBroadcast(long int networkId, TTPacket * pkt)
{
   // 1. FILTER packets from ourselves
   
   char * fromId = (char*)pkt->GetString("from");
   if ( !fromId ) {
      // malformed packet, no from field.
      TT_Error("TTNode::HandleBroadcast - no from field.");
      return;
   }
   else if ( strcmp(fromId, this_node->id) == 0 ) {
      // packet is from ourselves
      TT_Error("TTNode::HandleBroadcast - dropping self-routed packet");
      return;
   }
   
   // 2. FILTER DUPLICATE PACKETS / CACHE
   char * messageId = (char*)pkt->GetString("id");
   if ( !messageId ) return;  // malformed packet.
   int mid = atoi(messageId);
   if ( !packet_cache->Add(fromId, mid) ) {
      TT_Debug("TTNode::HandleBroadcast - packet %s:%i in cache", fromId, mid);
      return;
   }

   // broadcast to other nodes
   DoBroadcast(networkId, pkt);
   
   // handle the packet
}

// 
// Remote node is telling us that we're already connected, we
// need to disconnect the socket.

void TTNode::HandleRefused(long int networkId, TTPacket * pkt)
{
   char * fromId = (char*)pkt->GetString("from");
   if ( !fromId ) return;  // TODO: error message.
   TT_Debug("TTNod::HandleRefused(%li, packet) from %s", networkId, fromId);
   mutex->Lock();
   
   // remove the node from connections, if it is there.
   TTNodeInfo * ttni = (TTNodeInfo*)node_table->Get(fromId);
   if ( ttni ) {
      ttni->hops = 0;
      ttni->network_id = 0;
   }
   mutex->Unlock();
   
   // disconnect the network connection
   network->Disconnect(networkId);
}

//
// We are welcomed bya  peer onto the network.

void TTNode::HandleWelcome(long int networkId,TTPacket * pkt)
{
   char * fromId = (char*)pkt->GetString("from");
   if ( !fromId ) return;  // TODO: error message.
   
   TT_Debug("TTNod::HandleWelcome(%li, packet)", networkId);
   
   mutex->Lock();
   
   // see if the user is already logged in
   TTNodeInfo * ttni = (TTNodeInfo*)node_table->Get(fromId);

   if ( !ttni ) {
      // TODO: this is bad, disconnect node.
      mutex->Unlock();
      network->Disconnect(networkId);
   }
   else if ( ttni->hops == 1 ) {
      mutex->Unlock();
      // this could happen under certain circumstances.
      TTPacket * newPacket = new TTPacket();
      newPacket->route = TTPC_LOCAL;
      newPacket->AddString("type", "connection_refused");
      newPacket->AddString("from", this_node->id);
      newPacket->AddLong("id", current_packet_id++);
      network->SendPacket(networkId, newPacket);
      delete newPacket;
   }
   else {
      // new connection.
      ttni->network_id = networkId;
      ttni->hops = 1;
      mutex->Unlock();
      
      TT_Debug("TTNode::HandleWelcome: %s welcomed by %s on %li", this_node->id, ttni->id, networkId);
      
      // TODO: Add node to the connections list.
      connections->Add(ttni);
      
      // broadcast thew new arrival to the rest of the network.
      TTPacket * newPacket = new TTPacket();
      newPacket->route = TTPC_BROADCAST;
      newPacket->AddString("type", "node_on");
      newPacket->AddString("from", fromId);
      newPacket->AddString("id", (char*)pkt->GetString("id")); // using id of source packet.
      DoBroadcast(networkId, newPacket);
      delete newPacket;
   }
}

//
// Handle a login packet

void TTNode::HandleLogin(long int networkId, TTPacket * pkt)
{
   char * fromId = (char*)pkt->GetString("from");
   if ( !fromId ) return;  // TODO: error message.
   
   TT_Debug("TTNode::HandleLogin(%li, packet)", networkId);
   
   mutex->Lock();
   TTNodeInfo * ttni = (TTNodeInfo*)node_table->Get(fromId);
   if ( !ttni ) ttni = new TTNodeInfo(fromId);

   if ( ttni->hops == 1 || ttni->hops == -1 ) {
      mutex->Unlock();
      
      // already connected - send an error and disconnect
      TTPacket * newPacket = new TTPacket();
      newPacket->route = TTPC_LOCAL;
      newPacket->AddString("type", "connection_refused");
      newPacket->AddString("from", this_node->id);
      newPacket->AddLong("id", current_packet_id++);
      network->SendPacket(networkId, newPacket);
      delete newPacket;
   }
   else {
      ttni->network_id = networkId;
      ttni->hops = 1;
      mutex->Unlock();
      
      TTPacket * newPacket = new TTPacket();
      newPacket->route = TTPC_LOCAL;
      newPacket->AddString("type", "welcome");
      newPacket->AddString("from", this_node->id);
      newPacket->AddLong("id", current_packet_id++);
      network->SendPacket(networkId, newPacket);
      delete newPacket;
      
      TT_Debug("TTNode::HandleLogin: %s welcomes %s on %li", this_node->id, ttni->id, networkId);
      
      // TODO: add node to the connections list.
      connections->Add(ttni);
      
      // broadcast thew new arrival to the rest of the network.
      newPacket = new TTPacket();
      newPacket->route = TTPC_BROADCAST;
      newPacket->AddString("type", "node_on");
      newPacket->AddString("from", fromId);
      newPacket->AddString("id", (char*)pkt->GetString("id")); // using id of source packet.
      DoBroadcast(networkId, newPacket);
      delete newPacket;
   }
}

//
// Add a node to the list of nodes that we will potentially 
// connect to.

void TTNode::AddPeer(char * pPeer, int pPort)
{
   if ( this_node ) {
      TT_Debug("TTNode::AddPeer(%s, %i)",  pPeer, pPort); 
   }
   else {
      TT_Debug("TTNode::AddPeer(%s, %i)", pPeer, pPort); 
   }
   
   TTNodeInfo * newNode = new TTNodeInfo(pPeer, pPort);
   if ( !node_table->Put(newNode->id, (void*)newNode) ) {
      std::cout << "TTNode::AddPeer - node already exists in table" << std::endl;
      delete newNode;
   }
}

//
// Add a packet handler

void TTNode::AddHandler(TTHandler * pHandler)
{
   packet_handlers->Put(pHandler->Type(), (void*)pHandler);
}

//
// Add a packet listener.

void TTNode::AddListener(TTHandler * pHandler)
{
   packet_listeners->Insert((void*)pHandler);
}

//
// Find an eligable node and try to connect to it.  This function should 
// be called every n minutes.

void TTNode::ManageConnections()
{
   TT_Debug("TTNode::ManageConnections()");
   
   // if we have under < max_connections, send out a broadcast 
   // that we are available.
   TTPacket * newPacket = new TTPacket();
   newPacket->route = TTPC_BROADCAST;
   newPacket->AddString("from", this_node->id);
   newPacket->AddLong("id", current_packet_id++);
   newPacket->AddString("type", "node_info");
   // TODO: also include other info like # of connections, uptime, etc.
   DoBroadcast(0, newPacket);
   delete newPacket;
   
   // search for an available node to connect to.
   TTLinkedList * ttl = node_table->Enumerate();
   if ( ttl ) ttl = ttl->next;
   TTNodeInfo * ttni = NULL;
   while ( ttl && ttl->item ) {
      ttni = (TTNodeInfo*)ttl->item;
      if ( ConnectToNode(ttni) ) break;
 
      ttl = ttl->next;
   }
}

//
// Initiate a connection to another node.  Returns true if the 
// initiation is sucessful.  This doesn't mean we're connected yet, 
// we need to wait for the 'welcome' message to know that.

bool TTNode::ConnectToNode(TTNodeInfo * ttni)
{
   mutex->Lock();
   if ( ttni->hops == -1 ) {
      // we're already trying to connect to the node.
      mutex->Unlock();
      return false;
   }
   else if ( ttni->hops == 1 ) {
      // we're already connected to this node.
      mutex->Unlock();
      return false;
   }
   else {
      ttni->hops = -1;
      mutex->Unlock();
      
      long int netId = network->Connect(ttni->ip, ttni->port);
      ttni->network_id = netId;
      TT_Debug("TTNode::ConnectToNode: %s connecting to %s on %li", this_node->id, ttni->id, netId);
      TTPacket * pkt = new TTPacket();
      pkt->route = TTPC_LOCAL;
      pkt->AddString("type", "login");
      pkt->AddString("from", this_node->id);
      pkt->AddLong("id", current_packet_id++);
      network->SendPacket(netId, pkt);
      delete pkt;
      return true;
   }
}

//
// Broadcast the packet to all connected nodes, optionally excluding 
// *excludeId*. 

void TTNode::DoBroadcast(long int excludeId, TTPacket * pkt)
{
   // TODO: Might need to be mutex'd.
   TTLinkedList * tempList = connections->next;
   TTNodeInfo * ttni = NULL;
   
   while ( tempList ) { 
      ttni = (TTNodeInfo*)tempList->item;
      if ( ttni->network_id != excludeId ) {
         TT_Debug("TTNode::DoBroadcast - broadcasting to %s", ttni->id);
         network->SendPacket(ttni->network_id, pkt);
      }
      tempList = tempList->next;
   }
}
