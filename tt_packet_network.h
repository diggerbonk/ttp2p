//
// File     : $Id$
// Author   : Trent McNair
//
// TTPacketNetwork - class representing a network that 
// communicates via TTPackets.

#ifndef __tt_packet_network_h
#define __tt_packet_network_h

#include "ttools/tt_network.h"

class TTPacket;

class TTPacketNetwork : public TTNetwork {

public:

   TTPacketNetwork(TTNotify * ttpn);
   ~TTPacketNetwork();
   
   bool SendPacket(long int channel, TTPacket * pkt);
   virtual void DoNotify(long int channel, int type, void * data);
};

#endif // __tt_packet_network_h
   
