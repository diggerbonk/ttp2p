//
// File     : $Id$
// Author   : Trent McNair

#include <iostream>
#include <string.h>
#include <stdlib.h> // for atoi
#include <stdio.h>  // for sprintf
#include "ttools/tt_functions.h"
#include "tt_node_info.h"

using namespace std;

TTNodeInfo::TTNodeInfo(char * nodeId)
{
   ip = NULL;
   id = NULL;
   port = -1;
   alias = NULL;
   type = TT_NOTE_TYPE_UNK;
   hops = 0;
   network_id = -1;
   SetId(nodeId);
}

TTNodeInfo::TTNodeInfo(char * pip, int pport)
{
   ip = NULL;
   id = NULL;
   port = -1;
   alias = NULL;
   type = TT_NOTE_TYPE_UNK;
   hops = 0;
   network_id = -1;
   SetId(pip, pport);
}

TTNodeInfo::~TTNodeInfo()
{
   delete id;
   delete alias;
   delete ip;
}

void TTNodeInfo::SetId(char * pId)
{
   delete id;
   id = new char[strlen(pId)+1];
   strcpy(id,pId);
   
   delete ip;
   ip = new char[strlen(pId)+1];
   strcpy(ip,pId);
   
   char * scratch = strstr(ip,":");
   if ( !scratch ) return;
   scratch[0] = 0;
   scratch++;
   port = atoi(scratch);
}

void TTNodeInfo::SetId(char * pIp, int pPort)
{
   delete ip;
   ip = new char[strlen(pIp)+1];
   strcpy(ip,pIp);
   
   port = pPort;
   
   delete id;
   id = new char[strlen(pIp)+8];
   sprintf(id,"%s:%i", pIp, pPort);
}

void TTNodeInfo::SetAlias(char * pAlias)
{
   delete alias;
   alias = new char[strlen(pAlias)+1];
   strcpy(alias,pAlias);
}
