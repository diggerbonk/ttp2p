//
// File     : $Id$
// Author   : Trent McNair
//

#include "tt_connections.h"
#include "tt_node_info.h"

TTConnections::TTConnections() : TTLinkedList()
{
   number_of_connections = 0;
}

TTConnections::TTConnections(void * itm) : TTLinkedList(itm)
{
   number_of_connections = 0;
}

bool TTConnections::Add(TTNodeInfo * ttni)
{
   number_of_connections++;
   Insert((void*)ttni);
   return true;
}

bool TTConnections::Remove(long int networkId)
{
   TTLinkedList * nodeList = this;
   TTNodeInfo * ttni;
   TTLinkedList * ptr;
      
   while ( nodeList->next && nodeList->next->item ) {
      ttni = (TTNodeInfo*)nodeList->next->item;
      if ( ttni->network_id == networkId ) {
         // we don't have to delete the node info here because
         // it remains in the node_table.
         ttni->hops = 0;
         ttni->network_id = 0;
         ptr = nodeList->next;
         nodeList->next = nodeList->next->next;
         delete ptr;
         number_of_connections--;
      }
      nodeList = nodeList->next;
   }
   return true;
}

   
