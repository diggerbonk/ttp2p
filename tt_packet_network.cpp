//
// File     : $Id$
// Author   : Trent McNair
//
// TTPacketNetwork - class representing a network that 
// communicates via TTPackets.

#include <iostream>

#include "ttools/tt_buffer.h"
#include "ttools/tt_functions.h"
#include "ttools/tt_network.h"

#include "tt_packet_network.h"
#include "tt_packet.h"

using namespace std;

TTPacketNetwork::TTPacketNetwork(TTNotify * ttpn) : TTNetwork(ttpn)
{
}

TTPacketNetwork::~TTPacketNetwork() 
{
}

//
// Notification callback.

void TTPacketNetwork::DoNotify(long int channel, int type, void * data)
{
   if ( type == TT_NOTIFY_IN ) {

      // data is in on the network, we will try and parse a 
      // packet from whats available, if we don't get one, 
      // we'll just leave the buffer intact for when more data 
      // comes in.

      TTBuffer * tbuf = (TTBuffer*)data;
      TTPacket * pkt = new TTPacket();
      int bytesPopped = pkt->Objectify(tbuf->Buffer(),tbuf->Size());
      if ( bytesPopped > 0 ) {
         tbuf->Pop(bytesPopped);
         TT_Debug("TTPacketNetwork - packet in");
//         pkt->Print();
         notify->Notify(channel,type,(void*)pkt);
      }
      delete pkt;
   }
   else  {

      // we'll have TTNetwork handle anything that isn't 
      // data.

      TTNetwork::DoNotify(channel, type, data);
   }
}

bool TTPacketNetwork::SendPacket(long int channel, TTPacket * pkt)
{
   TTBuffer * tbuf = pkt->Serialize();
   if ( !tbuf ) return false;
   Send(channel, tbuf->Buffer(), tbuf->Size());
   delete tbuf;
   return true;
}
