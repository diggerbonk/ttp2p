//
// File     : $Id$
// Author   : Trent McNair

#include <iostream>
#include <string.h>
#include <time.h>
#include "tt_packet_cache.h"

using namespace std;

PacketItem::PacketItem(char * pId, int pSequence)
{
   if ( pId ) {
      id = new char[strlen(pId)+1];
      strcpy(id,pId);
      timestamp = time(NULL);
   }
   else {
      id = NULL;
      timestamp = 0;
   }
   
   next = NULL;
   last = NULL;
   
   sequence = pSequence;
}

TTPacketCache::TTPacketCache()
{
   head = NULL;
   tail = NULL;
}

//
// Add an item to the cache.  If teh item is already 
// in the cache, false is returned.

bool TTPacketCache::Add(char * pId, int pSequence)
{
   if ( !head ) {
      head = new PacketItem(pId,pSequence);
      list_size++;
      head->last = NULL;
      head->next = NULL;
      tail = head;
   }
   else {
      // search for item
      PacketItem * ptr = head;
      while ( ptr ) {
         if ( ptr->id && (ptr->sequence == pSequence) ) {
            if ( strcmp(ptr->id, pId) == 0 ) return false;
         }
         ptr = ptr->next;
      }
      
      // add the item
      ptr = new PacketItem(pId, pSequence);
      list_size++;
      
      // if we're over the list size max, clip the top
      while ( list_size > TT_PACKET_CACHE_MAX ) {
      }
   }
   
   return true;
}

TTPacketCache::~TTPacketCache()
{
}

void TTPacketCache::Print()
{
   PacketItem * pi = head;
   
   cout << "PacketCache Contents:" << endl;

   while ( pi && pi->id ) {
      cout << "   Item: " << pi->id << ", " << pi->sequence << endl;
      pi = pi->next;
   }
}

