//
// File     : $Id$
// Author   : Trent McNair
//
// TTNode implements the basic functionality of a node on the TTP2P network.

#ifndef __tt_node_h
#define __tt_node_h

#include "ttools/tt_notify.h"

const int TT_NODE_STATUS_OFF = 0;
const int TT_NODE_STATUS_ON = 1;
const int TT_NODE_STATUS_SHUTDOWN = 2;

const int TT_DEFAULT_PORT = 2541;

const int TT_DEFAULT_MAX_CONNECTIONS = 100;
const int TT_DEFAULT_MIN_CONNECTIONS = 10;

class TTHashtable;
class TTPacketCache;
class TTLinkedList;
class TTNodeInfo;
class TTMutex;
class TTPacketNetwork;
class TTHandler;
class TTNodeTable;
class TTConnections;

class TTNode : TTNotify {

public:

   TTNode();
   ~TTNode();
   
   bool Start(char * pIp, int pPort, char * pAlias);
   void AddPeer(char * ip, int port);
   void AddHandler(TTHandler * pHandler);
   void AddListener(TTHandler * pHandler);
   
   virtual void DoNotify(long int channel, int type, void * data);

protected:

   void ManageConnections();
   
   // route handlers
   void HandleInternal(long int id, TTPacket * pkt);
   void HandleLocal(long int id, TTPacket * pkt);
   void HandlePeer(long int id, TTPacket * pkt);
   void HandleDiscover(long int id, TTPacket * pkt);
   void HandleBroadcast(long int id, TTPacket * pkt);
   
   // Local packet handlers.
   void HandleLogin(long int id, TTPacket * pkt);
   void HandleWelcome(long int id, TTPacket * pkt);
   void HandleRefused(long int id, TTPacket * pkt);

   bool ConnectToNode(TTNodeInfo *);
   void DoBroadcast(long int excludeNetworkId, TTPacket * pkt);

private:

   TTNodeTable * node_table;
   TTHashtable * packet_handlers;
   TTLinkedList * packet_listeners;
   TTPacketNetwork * network;
   TTPacketCache * packet_cache;
   TTConnections * connections;
   
   TTNodeInfo * this_node;
   long int current_packet_id;
   int status;
   
   char * listen_ip;
   int listen_port;
   
   TTMutex * mutex;
   
   int max_connections;
   int min_connections;
};

#endif // __tt_node_h
