//
// File     : $Id$
// Author   : Trent McNair
//

#ifndef __tt_connections_h
#define __tt_connections_h

#include "ttools/tt_linked_list.h"

class TTNodeInfo;

class TTConnections : public TTLinkedList
{
public:
   TTConnections();
   TTConnections(void * itm);
   
   bool Add(TTNodeInfo*);
   bool Remove(long int);
   
private:

   int number_of_connections;
};

#endif
