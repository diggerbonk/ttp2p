//
// tt_packet.cpp
//
// Class encapsulating a TTPacket.  A TTPacket 
// is a linked list of name/value pairs along with 
// other packet info that can be serialized (not 
// in the java sense) and sent over the network.  TTNodes 
// communicate via TTPackets.
//
// This class contains methods to build a packet, as well 
// parse serialized packets.
//
// Author   : Trent McNair 
// Update   : $Date$
// Version  : $Revision$

#include <iostream>
#include <string.h>
#include <stdio.h> // for sprintf - try to get rid of it.

#include "tt_packet.h"
#include "ttools/tt_buffer.h"
#include "ttools/tt_functions.h"

using namespace std;

TTPacketList::TTPacketList(char * nm, unsigned char * vl, int plen)
{
   if ( nm ) {
      name = new char[strlen(nm)+1];
      strcpy(name,nm);
   }
   else name = NULL;
   if ( vl ) {
      value = new unsigned char[plen];
      memcpy(value, vl, plen);
      len = plen;
   }
   else {
      value = NULL;
      len = 0;
   }
   
   next = NULL;
}

TTPacketList::~TTPacketList()
{
   delete [] name;
   delete [] value;
   
   next = NULL;
}

TTPacket::TTPacket(/*long int pid*/)
{
//   id = pid;
   hops = 0;
   route = TTPC_PEER;
   packet_list = NULL;
}

TTPacket::~TTPacket()
{
   TTPacketList * ptr = packet_list;
   while ( ptr ) {
      packet_list = packet_list->next;
      delete ptr;
      ptr = packet_list;
   }
}

//
// Add a buffer value to the packet.

void TTPacket::Add(char * nm, unsigned char * val, int len)
{
   TTPacketList * pl = new TTPacketList(nm,val,len);
   pl->next = packet_list;
   packet_list = pl;
}

//
// Add
//
// Add a string to the packet.  If pTerminate is true, a null 
// terminator will be added to the end of the string.

void TTPacket::AddString(char * nm, char * val)
{
   Add(nm,(unsigned char*)val,strlen(val)+1);
}

void TTPacket::Clear()
{
   TTPacketList * ptr = packet_list;
   while ( ptr ) {
      packet_list = packet_list->next;
      delete ptr;
      ptr = packet_list;
   }
   packet_list = NULL;
}

//
// AddLong
//
// Convenience wrapper to add a long integer (as a string) 

void TTPacket::AddLong(char * nm, long int val)
{
   char str[16];
   sprintf(str, "%li", val);
   AddString(nm, str);
}

//
// Get.  Get the name value pair that matches name.  Returns 
// NULL if no match is found.

TTPacketList * TTPacket::Get(char * nm)
{
   TTPacketList * ptr = packet_list;
   while ( ptr && ptr->name ) {
      if ( strcmp(nm, ptr->name) == 0 ) return ptr;
      ptr = ptr->next;
   }
   return NULL;
}

//
// Return a string associated with nm.

const char * TTPacket::GetString(char * nm)
{
   TTPacketList * ptr = packet_list;
   while ( ptr && ptr->name ) {
      if ( strcmp(nm, ptr->name) == 0 ) return (const char*)ptr->value;
      ptr = ptr->next;
   }
   return NULL;
}

void TTPacket::Print()
{
   cout << "PACKET:" << endl;
   cout << "   route  : " << route << endl;
   cout << "   hops   : " << hops << endl;
   TTPacketList * pl = packet_list;
   while ( pl ) {
      cout << "      KEY : " << pl->name << endl;
      if ( pl->value ) {
         cout << "      VAL : " << pl->value << endl;
      }
      pl = pl->next;
   }
}

//
// Serialize - return a serialized version of the 
// packet - user is responsible for free'ing the 
// memory used in creating the buffer.  Returns 
// null if there's not a valid packet.

TTBuffer * TTPacket::Serialize()
{
   TTBuffer * ttbuf = new TTBuffer();
   ttbuf->AddString("*", false);
   ttbuf->AddByte((unsigned char)hops);
   ttbuf->AddByte((unsigned char)route);
   ttbuf->AddByte((unsigned char)0);
   ttbuf->AddShort(0);
   
   // Now we walk through and add each name/value pair.
   
   TTPacketList * pl = packet_list;
   while ( pl && pl->name && pl->value ) {
      ttbuf->AddShort((unsigned short)strlen(pl->name));
      ttbuf->AddString(pl->name,false);
      ttbuf->AddShort((unsigned short)pl->len);
      ttbuf->Add(pl->value, pl->len);
      pl = pl->next;
   }
   
   ttbuf->InsertShort((unsigned short)(ttbuf->Size()-6), 4);
   return ttbuf;
}

//
// Objectify - takes a serialized packet and converts it 
// to a packet object.  Returns the number of bytes 
// pulled from the buffer, or 0 if nothing is taken.

int TTPacket::Objectify(unsigned char * buf, int buflen/*, long int pid*/)
{
   if ( !buf || buflen < 6 ) return 0;
   
//   id = pid;
   
   int bytesLeft = buflen;
   unsigned char * buffer = buf;
   
   int len = TT_ShortFromBuffer(buffer,4) + 6;
   int bytesUsed = len;
   
   if ( bytesLeft < len ) return 0;
   
   hops = (int)buffer[1];
   route = (int)buffer[2];
   
   buffer += 6;
   len -= 6;
   
   // walk through the rest and add the name/value pairs
   int keyLen = 0;
   int valueLen = 0;
   char * key = NULL;
   
   while ( len > 0 ) {
      keyLen = TT_ShortFromBuffer(buffer, 0);
      if ( keyLen > len || keyLen <= 0 ) {
         // malformed frame
         TT_Debug("TTPacket.Objectify: Malformed frame");
         return 0;
      }
      else {
         len -= 2;
         buffer += 2;
         
         key = TT_StringFromBuffer(buffer, 0, keyLen);
         if ( key ) {
            len -= keyLen;
            buffer += keyLen;
         }
         else {
            TT_Debug("TTPacket.Objectify: Malformed frame");
            return 0;
         }
      }
      
      valueLen = TT_ShortFromBuffer(buffer, 0);
      if ( valueLen > len || valueLen < 0 ) {
         // malformed frame
         TT_Debug("TTPacket.Objectify: Malformed frame");
         delete [] key;
         key = NULL;
         return 0;
      }
      else if ( valueLen == 0 ) {
         // empty field.
         len-=2;
         buffer += 2;
         Add(key,NULL, 0);
      }
      else {
         len -= 2;
         buffer += 2;
         Add(key,buffer,valueLen);
         len -= valueLen;
         buffer += valueLen;
      }
      delete [] key;
      key = NULL;
   }
   
   return bytesUsed;
}

