//
// File     : $Id$
// Author   : Trent McNair
//
// TTHandler.  An interface for packet handlers.  Handler should 
//    return true if the packet was handled, false if not.

class TTPacket;

class TTHandler
{
public:

   virtual bool Handler(TTPacket * pkt);
   virtual char * Type();

};
