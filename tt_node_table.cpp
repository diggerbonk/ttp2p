//
// File     : $Id$
// Author   : Trent McNair
//
// TTNodeTable - modified hash table for handling nodes.

#include "tt_node_table.h"
#include "ttools/tt_linked_list.h"
#include "tt_node_info.h"

TTNodeTable::TTNodeTable(int max) : TTHashtable(max)
{
}

void TTNodeTable::ClearNetworkID(long int networkId)
{
   TTLinkedList * nodeList = Enumerate();
   TTNodeInfo * ttni;
   while ( nodeList ) {
      nodeList = nodeList->next;
      if ( nodeList && nodeList->item ) {
         ttni = (TTNodeInfo*)nodeList->item;
         if ( ttni->network_id == networkId ) {
            ttni->network_id = 0;
            ttni->hops = 0;
         }
      }
   }
}
