
#include <iostream>
#include <stdlib.h>
#include <sys/wait.h>

#ifdef WIN32
#else
#include <unistd.h>
#endif

#include "ttools/tt_notify.h"
#include "tt_packet_cache.h"
#include "tt_node_info.h"
#include "tt_packet.h"
#include "tt_packet_network.h"
#include "tt_node.h"

using namespace std;

const int TT_TEST_PACKET_CACHE = 0;
const int TT_TEST_NODE_INFO = 1;
const int TT_TEST_PACKET_NETWORK = 2;
const int TT_TEST_ADDNODE = 3;


class MyNotify : public TTNotify {
public:
   void DoNotify(long int pChannel, int pType, void * pData);
};

void MyNotify::DoNotify(long int pChannel, int pType, void * pData)
{
   cout << "MyNotify::DoNotify(" << pChannel << ", ";
   cout << pType << ", " << pData << ")" <<  endl;
    // dont do anything just yet.

   if ( pType == TT_NOTIFY_IN ) {
      TTPacket * pkt = (TTPacket*)pData;
      pkt->Print();
   }
}

void TestPacketCache()
{
}

void TestNodeInfo()
{
   TTNodeInfo * ttni = new TTNodeInfo("127.0.0.1", 2541);
   
   cout << ttni->ip << ", " << ttni->id << ", " << ttni->port << endl;
   
   
   ttni->SetId("127.0.0.1:2451");
   cout << ttni->ip << ", " << ttni->id << ", " << ttni->port << endl;
}

void TestPacketNetwork()
{
   TTNotify * notify = new MyNotify();
   TTPacketNetwork * ttpn = new TTPacketNetwork(notify);
   
   ttpn->Listen(NULL,2541);

   sleep(1);

   long int c1 = ttpn->Connect("127.0.0.1", 2541);

   TTPacket * pkt = new TTPacket();
   pkt->AddString("one", "this is one");
   pkt->AddString("two", "this is two");
   pkt->AddString("three", "this is three");
   pkt->AddString("four", "this is four");

   ttpn->SendPacket(c1, pkt);
   while ( true ) {
      usleep(1);
   }
}

void TestAddNode()
{
   TTNode * tn = new TTNode();
   tn->AddPeer("192.168.1.2", 2541);
}

void TestDupNode()
{
   TTNode * node1 = new TTNode();
   TTNode * node2 = new TTNode();
   TTNode * node3 = new TTNode();
   
   node1->AddPeer("192.168.1.2", 2543);
   node2->AddPeer("192.168.1.2", 2541);
   node3->AddPeer("192.168.1.2", 2542);
   
   node1->Start("192.168.1.2", 2541, "node1");
   usleep(100000);
   node2->Start("192.168.1.2", 2542, "node2");
   usleep(100000);
   node3->Start("192.168.1.2", 2543, "node3");
   
   while ( true ) {
      usleep(1);
   }
}

void TestNode(char * cport, char * rip, char * rp)
{
   std::cout << "Starting node on port " << cport << std::endl;
   std::cout << "Adding remote node " << rip << ":" << rp << std::endl;
   
   TTNode * node1 = new TTNode();
   node1->AddPeer(rip, atoi(rp));
   node1->Start( "0.0.0.0" , atoi(cport), "MyNode");
   while ( true ) sleep(1);
}

void TestNull()
{
   cout << atoi(NULL) << endl;
}

int main ( int argc, char * argv[] )
{
   if ( argc < 2 ) {
      std::cout << "command requires an argument" << std::endl;
   }
   else if ( strcmp(argv[1], "pcache") == 0 ) {
      TestPacketCache();
   }
   else if ( strcmp(argv[1], "info") == 0 ) {
      TestNodeInfo();
   }
   else if ( strcmp(argv[1], "pnetwork") == 0 ) {
      TestPacketNetwork();
   }
   else if ( strcmp(argv[1], "addnode") == 0 ) {
      TestAddNode();
   }
   else if ( strcmp(argv[1], "dupnode") == 0 ) {
      TestDupNode();
   }
   else if ( strcmp(argv[1], "null") == 0 ) {
      TestNull();
   }
   else if ( strcmp(argv[1], "node") == 0 ) {
      if ( argc < 5 ) {
         std::cout << "Useage: testapp node <port> <remote ip> <remote port>" << std::endl;
         return 0;
      }
      TestNode(argv[2], argv[3], argv[4]);
   }
}
