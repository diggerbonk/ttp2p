//
// File     : $Id$
// Author   : Trent McNair
//
// TTNodeTable - modified hash table for handling nodes.

#ifndef __tt_node_table_h
#define __tt_node_table_h

#include "ttools/tt_hashtable.h"

class TTNodeTable : public TTHashtable
{
public:
   TTNodeTable(int size);
   
   void ClearNetworkID(long int nid);
};
#endif


