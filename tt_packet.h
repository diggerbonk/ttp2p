//
// tt_packet.h
//
// Class encapsulating a TTPacket.  A TTPacket 
// is a linked list of name/value pairs along with 
// other packet info that can be serialized (not 
// in the java sense) and sent over the network.  TTNodes 
// communicate via TTPackets.
//
// This class contains methods to build a packet, as well 
// parse serialized packets.
//
// Author   : Trent McNair 
// Update   : $Date$
// Version  : $Revision$

#ifndef __tt_packet_h
#define __tt_packet_h

// PACKET CATEGORIES

#define TTPC_INTERNAL  0
#define TTPC_LOCAL     1
#define TTPC_PEER      2
#define TTPC_DISCOVER  3
#define TTPC_BROADCAST 4

class TTBuffer;

//
// TTPacketList
//
// Just a node for a linked list of name value pairs. 

class TTPacketList
{

public:
   TTPacketList(char * nm, unsigned char * vl, int len);
   ~TTPacketList();

   char * name;
   unsigned char * value;
   int len;
   TTPacketList * next;
};

class TTPacket
{
public:
   TTPacket(/*long int pid = -1*/);
   ~TTPacket();
   
   void Add(char * nm, unsigned char * val, int len);
   void AddString(char * nm, char * str);
   void AddLong(char * nm, long int num);
   TTPacketList * Get(char * nm);
   const char * GetString(char * nm);
   TTPacketList * List(){return packet_list;}
   void Clear();
   void Print();
   
   TTBuffer * Serialize();
   int Objectify(unsigned char * buf, int len/*, long int pid = -1*/);

//private:

//   long int id;
   int hops;
   int route; // internal, local, peer, discover, broadcast
   TTPacketList * packet_list;
};
#endif // __tt_packet_h
