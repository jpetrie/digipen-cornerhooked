/*! ========================================================================

      @file    NetServer.cpp
      @author  jmp
      @brief   Implementation of server networking.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================  */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "NetServer.h"

#include "NetPackets.h"

#include "Game.h"
#include "GameSession.h"
#include "UIButton.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  // slot codes
  const int  kSlotAvail  = -1;  //!< Empty and available.
  const int  kSlotClosed = -2;  //!< Empty and unavailable (host has closed it).
}


/*                                                                 variables
---------------------------------------------------------------------------- */

//namespace
//{
  NetServerData *gServer = 0;
//}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
void InitServer(NetServerData *data)
/*! Initialize server data.

    @param data  A pointer to a NetServerData object that will store server
                 data. The caller is responsible for allocating memory for
                 this object and for releasing it after invoking KillServer().
*/
{
  if (0 != gServer)
	  KillServer() ;
  
  // Create a socket to listen for incoming connections.
  data->listenSock = socket(AF_INET,SOCK_STREAM,0);
  ENFORCE(data->listenSock != SOCKET_ERROR)("Failed to create listen socket.");

  data->listenAddr.sin_family      = AF_INET;
  data->listenAddr.sin_port        = htons(kNetGamePort);
  data->listenAddr.sin_addr.s_addr = INADDR_ANY;
  ::memset(&(data->listenAddr.sin_zero),0,8);

  ENFORCE(SOCKET_ERROR != bind(data->listenSock,reinterpret_cast< sockaddr* >(&data->listenAddr),sizeof(data->listenAddr)))
         ("Failed to bind listen socket.");
  ENFORCE(SOCKET_ERROR != listen(data->listenSock,kNetServerBacklog))
         ("Failed to activate listen socket.");
  ENFORCE(SOCKET_ERROR != ::WSAAsyncSelect(data->listenSock,Game::Get()->GetWindow()->GetHandle(),GM_NETSERVER_ACCEPT,FD_ACCEPT))
         ("Failed to enable async read on listen socket.");

  // IDs will start from 0.
  data->lastIDAssigned = -1;
  
  // Initially, all slots are empty and available.
  for(unsigned int i = 0; i < 8; ++i)
    data->peerSlots[i] = kSlotAvail;

  // Save the address.
  gServer = data;
}

/*  ________________________________________________________________________ */
void KillServer(void)
/*! Release server data.
    
    This function will release any resources created by the server, but the
    caller is still responsible for releasing the NetServerData object passed
    to InitServer().
*/
{
  if (0 != gServer)
  {
	// Close the listen socket and terminate all connections.
	std::map< SOCKET,Connection >::iterator  it;

	for(it = gServer->pendList.begin(); it != gServer->pendList.end(); ++it)
		closesocket(it->first);
	gServer->pendList.clear() ;
	for(it = gServer->peerList.begin(); it != gServer->peerList.end(); ++it)
		closesocket(it->first);
	gServer->peerList.clear() ;
	closesocket(gServer->listenSock);

	// Zero the pointer.
	gServer = 0;
  }
}

/*  ________________________________________________________________________ */
void NetServerAcceptPending(SOCKET fromSock)
/*! Accept a connection from the specified socket to the pending list.

    Remote machines remain pending until they time out or send login
    information, at which point they become peer connections.
*/
{
sockaddr_in  remoteAddr;
int          remoteAddrSz = sizeof(remoteAddr);
SOCKET       remoteSock   = accept(fromSock,reinterpret_cast< sockaddr* >(&remoteAddr),&remoteAddrSz);
Connection   pending;

  ENFORCE(SOCKET_ERROR != remoteSock)
         ("Failed to accept incoming connection.");
  ENFORCE(SOCKET_ERROR != ::WSAAsyncSelect(remoteSock,Game::Get()->GetWindow()->GetHandle(),GM_NETSERVER_READ,FD_READ))
         ("Failed to enable async read on new connection.");

  // Copy over information.
  pending.sock    = remoteSock;
  pending.port    = remoteAddr.sin_port;
  pending.address = inet_ntoa(remoteAddr.sin_addr);
  gServer->pendList.insert(std::make_pair(pending.sock,pending));
}

/*  ________________________________________________________________________ */
void NetServerHandleJoin(SOCKET sock,const char *buffer,size_t size)
/*! Unmarshall and handle join packet.
*/
{
PacketJoin    p;
nsl::bstream  stream;
char          id;

  // First, unmarshall the packet.
  ASSERT(*buffer == PacketJoin::ID);
  stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),size);
  stream >> id >> p.playerName;
  
  // Find the socket in the pending list, remove it, and make it a peer.
std::map< SOCKET,Connection >::iterator  it = gServer->pendList.find(sock);

  // Update the peerSlots status
  for ( unsigned int i = 0 ; i < kPlayersMax ; ++i )
  {
    if ( gServer->peerSlots[ i ] == kSlotAvail || gServer->peerSlots[ i ] == kSlotClosed )
    {
      UIButton * b = scast< UIButton * >( scast< UIPanel * >( Game::Get()->GetScreen( "GameOptions" )->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOPKickButtonName[ i ] ) ) ;
	  if ( b->GetCaption() == kUI_GOKickBtnCap || b->GetCaption() == kUI_GOOpenBtnCap )
        gServer->peerSlots[ i ] = kSlotClosed ;  // AI Player, Human Player, or Closed
      else // b->GetCaption() == kUI_GOCloseBtnCap
        gServer->peerSlots[ i ] = kSlotAvail ;   // Available
    }
  }

  // If the socket isn't in the pending list we'll silently ignore
  // this packet, basically.
  if(it != gServer->pendList.end())
  {
  bool  completed = false;
  
    // Store login information.
//    it->second.id   = ++gServer->lastIDAssigned;
    it->second.name = p.playerName;
    
    // Find the first available slot and shove it in there.
    for(unsigned int i = 0; i < 8; ++i)
    {
      if(gServer->peerSlots[i] == kSlotAvail)
      {
        gServer->peerSlots[i] = it->second.id = gServer->lastIDAssigned = i;

        // The connection was completed.
        completed = true;
        break;
      }
    }
    
    if(completed)
    {
      gServer->peerList.insert(std::make_pair(it->first,it->second));
      gServer->pendList.erase(it);
    }
    else
    {
      ///@todo inform pending connection it cannot join
    }
  }

  PacketGameOptions  po;

  // Update the game session information and broadcast the new stuff.
  po.gameName  = Game::Get()->GetSession()->GetGameName();
  po.playerCur = Game::Get()->GetSession()->GetPlayersCur() + 1;
  po.playerMax = Game::Get()->GetSession()->GetPlayersMax();
  for(int i = 0; i < kPlayersMax; ++i)
  {
  PacketGameOptions::PlayerInfo  info;
  
    if(i == gServer->lastIDAssigned)
    {
      // This is the guy that just joined.
      info.name = p.playerName;
      info.type = kPlayerType_Human;
    }
    else
    {
      Player *p = Game::Get()->GetSession()->GetPlayer(i);
    
      // Pull the player info from the game.
      if(p == 0)
      {
		UIButton * b = scast< UIButton * >( scast< UIPanel * >( Game::Get()->GetScreen( "GameOptions" )->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOPKickButtonName[ i ] ) ) ;
		if ( b->GetCaption() == kUI_GOKickBtnCap || b->GetCaption() == kUI_GOOpenBtnCap )
		{
	      info.name = kUI_GOSlotCloseBtnCap;
		  info.type = kPlayerType_Closed;
		}
		else // b->GetCaption() == kUI_GOCloseBtnCap
		{
		  info.name = kUI_GOSlotAvailBtnCap;
          info.type = kPlayerType_Avail;
		}
      }
	  else if (p->IsAI())
	  {
		info.name = kUI_GOSlotAIBtnCap;
		info.type = kPlayerType_AI;
	  }
	  else
      {
        info.name = p->GetName();
        info.type = kPlayerType_Human;
      }
    }
    
    // Store info for this slot.
    po.players.push_back(info);
  }
  po.gameType = gCurrentGameType ;
  
  NetServerSendGameOptions(po);
}

/*  ________________________________________________________________________ */
void NetServerRebroadcast(const char *buffer,size_t sz)
/*! Rebroadcast packet data to all peers.
*/
{
std::map< SOCKET,Connection >::iterator  it = gServer->peerList.begin();

  // Send it to everybody.
  while(it != gServer->peerList.end())
  {
    send(it->second.sock,buffer,sz,0);
    ++it;
  }
}

/*  ________________________________________________________________________ */
void NetServerSendGameOptions(const PacketGameOptions &packet)
/*! Broadcast game options to all peers.
*/
{
std::map< SOCKET,Connection >::iterator  it = gServer->peerList.begin();
nsl::bstream                             buffer;

  // Marshall the packet.
  buffer << static_cast< char >(PacketGameOptions::ID)
         << packet.gameName
         << packet.playerCur  << packet.playerMax;
  for(unsigned int i = 0; i < packet.players.size(); ++i)
  {
  std::string  na = packet.players[i].name;
    buffer << packet.players[i].type << packet.players[i].name;
  }
  buffer << packet.gameType ;
  
  // Send it to everybody.
  while(it != gServer->peerList.end())
  {
    ::send(it->second.sock,(char*)buffer.data(),buffer.size(),0);
    ++it;
  }
}

/*  ________________________________________________________________________ */
void NetServerSendStart(void)
/*! Broadcast start packet to all peers.

    This function sends a start packet to all connected peers. Each packet
    contains the turn ID of the peer it is sent to.
*/
{
std::map< SOCKET,Connection >::iterator  it   = gServer->peerList.begin();
unsigned int                             turn = 0;

  while(it != gServer->peerList.end())
  {
  nsl::bstream  buffer;
  
    buffer << static_cast< char >(PacketGameStart::ID) << turn;
    send(it->second.sock,(char*)buffer.data(),buffer.size(),0);
    ++it;
    ++turn;
  }
}

/*  ________________________________________________________________________ */
void NetServerSendSync(const std::vector< D3DXVECTOR3 > &balls,const std::vector< char > &pflags)
/*! Broadcast end-of-turn sync packet to all peers.
*/
{
std::map< SOCKET,Connection >::iterator  it   = gServer->peerList.begin();
nsl::bstream  buffer;

  buffer << static_cast< char >(PacketEndTurnSync::ID) << (int)balls.size();
  for(unsigned int i = 0; i < balls.size(); ++i)
    buffer << balls[i].x << balls[i].y << balls[i].z;
  for(unsigned int i = 0; i < pflags.size(); ++i)
    buffer << pflags[i];
  
  while(it != gServer->peerList.end())
  {
    send(it->second.sock,(char*)buffer.data(),buffer.size(),0);
    ++it;
  }
}

/*  ________________________________________________________________________ */
void NetServerSendKick(int slot)
/*! Broadcast a kick player packet to the appropriate peer.
*/
{
  Player * pKickedPlayer = Game::Get()->GetSession()->GetPlayer( slot ) ;
  if ( pKickedPlayer == 0 )
	  return ;

    // If player is an AI, simply release it; otherwise apply the full monty
  if ( !pKickedPlayer->IsAI() )
  {
    nsl::bstream  buffer;
    buffer << static_cast< char >(PacketKick::ID) << slot;

    std::map< SOCKET,Connection >::iterator it = gServer->peerList.begin();
    while ( it != gServer->peerList.end() )
	{
	  if ( it->second.id == slot )
      {
	      // Send packet, and close socket
        send(it->second.sock,(char*)buffer.data(),buffer.size(),0);
        closesocket(it->second.sock);

  	      // Remove from peer list
        std::map< SOCKET,Connection >::iterator tempItr = it ;
		--it ;
        gServer->peerList.erase(tempItr);
      }
	  ++it ;
	}

    SAFE_DELETE( pKickedPlayer ) ;
  }

  // Remove player and update slot availability
  Game::Get()->GetSession()->SetPlayer( slot , 0 ) ;
  gServer->peerSlots[slot] = kSlotAvail;

  // Update UI properly
  UIButton * b ;
  UIPanel  * p = static_cast< UIPanel * >( Game::Get()->GetScreen( "GameOptions" )->GetElement( kUI_GOPanelName ) ) ;
  b = static_cast< UIButton * >( p->GetElement( kUI_GOPButtonName[slot] ) ) ;
  b->SetCaption( kUI_GOSlotAvailBtnCap ) ;
  b = static_cast< UIButton * >( p->GetElement( kUI_GOPKickButtonName[slot] ) ) ;
  b->SetCaption( kUI_GOCloseBtnCap ) ;
  b = static_cast< UIButton * >( p->GetElement( kUI_GOPAddAIButtonName[slot] ) ) ;
  b->Enable( true ) ;
}

/*  ________________________________________________________________________ */
void NetServerAddPlayer(int slot)
{
    PacketGameOptions  po;

    // Update the game session information and broadcast the new stuff.
    po.gameName  = Game::Get()->GetSession()->GetGameName();
    po.playerCur = Game::Get()->GetSession()->GetPlayersCur() + 1;
    po.playerMax = Game::Get()->GetSession()->GetPlayersMax();
	po.gameType  = gCurrentGameType ;

    // set the player in the active slot as an AI player
    //Game::Get()->GetSession()->SetPlayer(slot, Game::Get()->GetAIPlayer());

    // add all of the players to the list
    for(int i = 0; i < kPlayersMax; ++i)
    {
        Player *p = Game::Get()->GetSession()->GetPlayer(i);   
        PacketGameOptions::PlayerInfo  info; 
        if(i == slot)
        {
            info.name = kUI_GOSlotAIBtnCap;
            info.type = kPlayerType_AI;
        }
        else if(p == 0)
        {
			UIButton * b = scast< UIButton * >( scast< UIPanel * >( Game::Get()->GetScreen( "GameOptions" )->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOPKickButtonName[ i ] ) ) ;
			if ( b->GetCaption() == kUI_GOKickBtnCap || b->GetCaption() == kUI_GOOpenBtnCap )
			{
	            info.name = kUI_GOSlotCloseBtnCap;
		        info.type = kPlayerType_Closed;
			}
			else // b->GetCaption() == kUI_GOCloseBtnCap
			{
				info.name = kUI_GOSlotAvailBtnCap;
				info.type = kPlayerType_Avail;
			}
        }
		else if (p->IsAI())
		{
			info.name = kUI_GOSlotAIBtnCap;
			info.type = kPlayerType_AI;
		}
        else
        {
            info.name = p->GetName();
            info.type = kPlayerType_Human;
        }
        // Store info for this slot.
        po.players.push_back(info);
    }
    
  NetServerSendGameOptions(po);
}

/*  ________________________________________________________________________ */
void NetServerSendQuit(void)
//! Broadcast quit packet to all peers.
{
	if ( gServer == 0 )
		return ;

	nsl::bstream buffer ;
	buffer << scast< char >( PacketQuit::ID ) << scast< unsigned int >( Game::Get()->GetMyTurn() ) ;

	std::map< SOCKET , Connection >::iterator it = gServer->peerList.begin() ;
	while(it != gServer->peerList.end())
	{
		send( it->second.sock , rcast< const char * >( buffer.data() ) , buffer.size() , 0 ) ;
		++it ;
	}

	// For some reason this is necessary to get the packets out before the server exits
	Sleep( 100 ) ;
}
