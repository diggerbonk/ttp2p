//
// File     : $Id$
// Author   : Trent McNair
//
// TTPacketNotify : Interface class for receiving notifications from 
// a pakcket network.

class TTPacket;

const int TTPN_PACKET = 0;
const int TTPN_CONNECTING = 1;

class TTPacketNotify
{
public:

   virtual void DoNotify(long int channel, TTPacket * pkt) = 0;
   void Notify(long int channel, TTPacket * pkt);
   void DoNotifyLater(long int channel, TTPacket * pkt);
};
