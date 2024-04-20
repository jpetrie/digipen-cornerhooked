/*! ========================================================================

      @file    NetTracker.cpp
      @author  jmp
      @brief   Implementation of score tracker networking.
      
    ======================================================================== 
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "NetTracker.h"

#include "nsl_bstream.h"

/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
void UpdateTrackServer(void)
{
int f;
SOCKET       trackSock = socket(AF_INET,SOCK_STREAM,0);
sockaddr_in  trackAddr;

  trackAddr.sin_family      = AF_INET;
  trackAddr.sin_port        = htons(6240);
  trackAddr.sin_addr.s_addr = inet_addr("63.111.27.169");
  ::memset(&(trackAddr.sin_zero),0,8);

  // This also needs to be done async style!
  f = connect(trackSock,(sockaddr*)&trackAddr,sizeof(sockaddr));
  f = WSAGetLastError();
  
nsl::bstream  pack;

  pack << "POST jmp 32 END";
  
//int packet = htonl(12);

  f = send(trackSock,(char*)pack.data(),pack.size(),0);

  f = closesocket(trackSock);  
}