/*! ========================================================================

      @file    NetClient.cpp
      @author  jmp
      @brief   Implementation of client networking.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================  */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "NetClient.h"
#include "NetPackets.h"

#include "Game.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  NetClientData *gClient = 0;
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
void InitClient(NetClientData *data,const std::string &addr,short port)
/*! Initialize client data.
*/
{
  if ( gClient != 0 )
	  KillClient() ;

  // Create a socket to connect on.
  data->gameSock = socket(AF_INET,SOCK_STREAM,0);
  ENFORCE(data->gameSock != SOCKET_ERROR)("Failed to create game socket.");

  data->gameAddr.sin_family      = AF_INET;
  data->gameAddr.sin_port        = htons(port);
  data->gameAddr.sin_addr.s_addr = inet_addr(addr.c_str());
  ::memset(&(data->gameAddr.sin_zero),0,8);

///@todo renamed handler and message to something sane.
  ENFORCE(SOCKET_ERROR != ::WSAAsyncSelect(data->gameSock,Game::Get()->GetWindow()->GetHandle(),GM_NETCLIENT_CONNECT,FD_CONNECT | FD_READ))
         ("Failed to enable async connect on client socket.");
  connect(data->gameSock,reinterpret_cast< sockaddr* >(&data->gameAddr),sizeof(data->gameAddr));
  
  // Save this pointer.
  gClient = data;
}

/*!
	\brief	Close client's socket and free data
 */
void KillClient( void )
{
	if ( gClient != 0 )
	{
		closesocket( gClient->gameSock ) ;
		gClient = 0 ;
	}
}

/*  ________________________________________________________________________ */
void NetClientSendJoin(const std::string &playerName)
/*! Send join game packet.
*/
{
 ASSERT(0 != gClient);
  
nsl::bstream  packet;

  // Marshall.
  packet << static_cast< char >(PacketJoin::ID) << playerName;
  
  // Send.
  send(gClient->gameSock,reinterpret_cast< const char* >(packet.data()),packet.size(),0);  
}

/*  ________________________________________________________________________ */
void NetClientSendTurn(D3DXVECTOR3 direction,float power)
/*! Send join game packet.
*/
{
 ASSERT(0 != gClient);
 
nsl::bstream  packet;

  // Marshall and send.
  packet << static_cast< char >(PacketTurn::ID) << direction.x << direction.y << direction.z << power;
  send(gClient->gameSock,reinterpret_cast< const char* >(packet.data()),packet.size(),0);  
}

/*  ________________________________________________________________________ */
void NetClientSendChat(const std::string &msg)
/*! Send chat packet.

    @param msg  The chat message to send.
*/
{
  ASSERT(0 != gClient);
  nsl::bstream  packet;
  
  // Don't send empty chat.
  if(msg.empty())
    return;

std::stringstream  fmt;

  fmt << "(" << Game::Get()->GetMyName() << ")  " << msg;

  // Marshall and send.
  packet << static_cast< char >(PacketChat::ID) << fmt.str();
  send(gClient->gameSock,reinterpret_cast< const char* >(packet.data()),packet.size(),0);  
}

/*  ________________________________________________________________________ */
void NetClientSendCueAdjust(float dx,float dy,float dz)
/*! Send cue adjustment packet.

    @param dx  X adjust delta.
    @param dy  X adjust delta.
    @param dz  X adjust delta.
*/
{
  ASSERT(0 != gClient);
  
  // Marshall and send.
nsl::bstream  packet;

  packet << static_cast< char >(PacketCueAdjust::ID) << dx << dy << dz;
  send(gClient->gameSock,reinterpret_cast< const char* >(packet.data()),packet.size(),0);  

}

/*  ________________________________________________________________________ */
void NetClientSendQuit(void)
//! Send quit packet to server
{
	if ( gClient == 0 )
		return ;

	nsl::bstream buffer ;
	buffer << scast< char >( PacketQuit::ID ) << scast< unsigned int >( Game::Get()->GetMyTurn() ) ;

	send( gClient->gameSock , rcast< const char * >( buffer.data() ) , buffer.size() , 0 ) ;
}
