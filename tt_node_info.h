//
// File     : $Id$
// Author   : Trent McNair

#ifndef __tt_node_info_h
#define __tt_node_info_h

const int TT_NOTE_TYPE_UNK = 0;
const int TT_NODE_TYPE_PEER = 1;
const int TT_NODE_TYPE_LEAF = 2;

class TTNodeInfo {

public:

   TTNodeInfo(char * pid);
   TTNodeInfo(char * pip, int pport);
   ~TTNodeInfo();
   
   void SetId(char * pId);
   void SetId(char * pIp, int port);
   void SetAlias(char * pAlias);
      
   char * ip;
   char * id;
   int port;
   int type;
   char * alias;
   int hops;
   long int network_id;
};

#endif // __tt_node_info_h
