/*! ========================================================================

      @file    NetClient.h
      @author  jmp
      @brief   Interface to client networking.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================  */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _NET_CLIENT_H_
#define _NET_CLIENT_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "NetServer.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// event messages
const int GM_NETCLIENT_CONNECT = WM_USER + 20;
const int GM_NETCLIENT_READ    = WM_USER + 21;


/*                                                                   structs
---------------------------------------------------------------------------- */

struct NetClientData
//! Encapsulates data
{
  SOCKET       gameSock;  //!< Socket on which we're connected to the server.
  sockaddr_in  gameAddr;  //!< Address of the server.
};


/*                                                                prototypes
---------------------------------------------------------------------------- */

// init
void InitClient(NetClientData *data,const std::string &addr,short port);
void KillClient( void ) ;

// packet sending
void NetClientSendJoin(const std::string &playerName);
void NetClientSendTurn(D3DXVECTOR3 direction,float power);
void NetClientSendChat(const std::string &msg);
void NetClientSendCueAdjust(float dx,float dy,float dz);
void NetClientSendQuit(void);

#endif  /* _NET_CLIENT_H_ */