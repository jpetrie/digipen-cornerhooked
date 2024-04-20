/*! ========================================================================

      @file    NetServer.h
      @author  jmp
      @brief   Interface to server networking.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================  */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _NET_SERVER_H_
#define _NET_SERVER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "NetPackets.h"

#include "nsl_bstream.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// network port
const short  kNetGamePort = 7701;

// backlog
const int kNetServerBacklog = 8;

// event messages
const int GM_NETSERVER_ACCEPT = WM_USER + 10;
const int GM_NETSERVER_READ   = WM_USER + 11;


/*                                                                   structs
---------------------------------------------------------------------------- */

struct Connection
//!< Encapsulates pending connection information.
{
  SOCKET       sock;
  std::string  address;
  short        port;
  
  std::string  name;  //!< Player name (only peer connections).
  int          id;    //!< Player ID (only peer connections); not ordered.
};

struct NetServerData
//! Encapsulates server data.
{
  SOCKET       listenSock;  //!< Socket server listens on.
  sockaddr_in  listenAddr;  //!< Sockaddr for above socket.
  
  int  lastIDAssigned;  //!< Last assigned peer ID.
  
  std::map< SOCKET,Connection >  pendList;      //!< Pending connection list.
  std::map< SOCKET,Connection >  peerList;      //!< Peer connection list.
  int                            peerSlots[8];  //!< Peer availability information.
};


/*                                                                prototypes
---------------------------------------------------------------------------- */

// init
void InitServer(NetServerData *data);
void KillServer(void);

// accepting
void NetServerAcceptPending(SOCKET fromSock);

// handling packets
void NetServerHandleJoin(SOCKET sock,const char *buffer,size_t size);

// packet sending
void NetServerRebroadcast(const char *buffer,size_t size);
void NetServerSendGameOptions(const PacketGameOptions &packet);
void NetServerSendStart(void);
void NetServerSendSync(const std::vector< D3DXVECTOR3 > &balls,const std::vector< char > &pflags);
void NetServerSendKick(int slot);
void NetServerAddPlayer(int);
void NetServerSendQuit(void);

#endif  /* _NET_SERVER_H_ */