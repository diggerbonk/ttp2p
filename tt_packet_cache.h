//
// File     : $Id$
// Author   : Trent McNair

#ifndef __tt_packet_cache_h
#define __tt_packet_cache_h

const int TT_PACKET_CACHE_MAX = 1000;

class PacketItem {

public:

   PacketItem(char * pId, int pSequence);
   ~PacketItem();
   
   char * id;
   int sequence;
   PacketItem * next;
   PacketItem * last;
   long int timestamp;
};

class TTPacketCache {

public:

   TTPacketCache();
   ~TTPacketCache();
   
   void Print(); // DEBUGING

   bool Add(char * pId, int pSequence);
   
   PacketItem * head;
   PacketItem * tail;
   int list_size;
   
};

#endif // __tt_packet_cache_h
