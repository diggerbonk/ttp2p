//
// File     : $Id$
// Author   : Trent McNair
//
// TTPacketNotify - interface for recieving tt_packet_network
// notifications.

#include "tt_packet_notify.h"
#include "tt_packet.h"

//
// Call this function to send a notification. 

void TTPacketNotify::Notify(long int pChannel, TTPacket * pkt)
{
   // if we're a gui, well call DoNotifyLater(), otherwise we just do 
   // the notification now, in this thread context.

   DoNotify(pChannel, pkt);
}

void TTPacketNotify::DoNotifyLater(long int pChannel, TTPacket * pkt)
{
   // push the packet onto the event queue of the given system.
}
