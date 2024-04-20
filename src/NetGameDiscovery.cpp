/*! ========================================================================

      @file    NetGameDiscovery.cpp
      @author  jmp,jsl
      @brief   Implementation of game discovery.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "NetGameDiscovery.h"

#include "Window.h"

#include "Game.h"
#include "GameSession.h"

#include "nsl_bstream.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
//  const char		  kDiscoveryServiceName[] = "_chooked._udp";
	const std::string  kCornerHookedID         = "cornerHOOKED" ;
    const int		   kDiscoveryServicePort   = 7737;
    const int		   kBroadcastDelay         = 120;  // measured in NetGameRegUpdate() calls (which presumably correspond to game loops)
	const unsigned int kCullingDelay           = 480;  // measured in UpdateDiscoveryInterface() calls
}


/*                                                                  typedefs
---------------------------------------------------------------------------- */

//typedef std::pair< std::string,char* >   HNLookupTask;
//typedef std::map< HANDLE,HNLookupTask >  HNLookupTaskMap;


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
//  DNSServiceRef  gBroadcastRef;
//  DNSServiceRef  gBrowserRef;
  
//  std::map< SOCKET,DNSServiceRef >  gResolveRefs;
  
//  bool        gBroadcasting = false;

//  HNLookupTaskMap gHNLookupTasks;



      // Net Discovery general data
    static NetGameMap s_games;

      // Net Discovery client data (build game list)
    static SOCKET s_listenSocket ;

      // Net Discovery broadcast data (server advertisement)
    static SOCKET          s_bcSocket ;   // broadcast socket
    static struct sockaddr s_bcAddr ;     // broadcast address (port, etc)
    static int             s_bcDelay ;    // no need to broadcast every frame
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*****************************************************************************/
//-- InitDiscoveryInterface -------------------------------------------------//
/*!
 *  \brief    Initialize the discovery interface.
 */

void InitDiscoveryInterface( void )
{
      // socket setup
    s_listenSocket = socket( PF_INET , SOCK_DGRAM , 0 ) ;
    if ( s_listenSocket == INVALID_SOCKET )
        return ;  // run away~~!

    sockaddr_in  addr = { 0 };

    addr.sin_port = htons(kDiscoveryServicePort);
    addr.sin_addr.S_un.S_addr = INADDR_ANY;
    addr.sin_family = AF_INET;

    bind(s_listenSocket,(sockaddr*)(&addr),sizeof(addr));

      // Clear game list
    s_games.clear() ;
}

/*****************************************************************************/
//-- KillDiscoveryInterface -------------------------------------------------//
/*!
 *  \brief    Close down the discovery interface.
 */

void KillDiscoveryInterface( void )
{
    closesocket( s_listenSocket ) ;
}

/*****************************************************************************/
//-- UpdateDiscoveryInterface -----------------------------------------------//
/*!
 *  \brief    Update game list if there are new games to be "discovered"
 */

void UpdateDiscoveryInterface( void )
{
    struct timeval timeout = { 0 , 0 } ;  // no wait / immediate return
    fd_set         readable ;

      // Set up fd_set to check if anything can be read from s_listenSocket
    FD_ZERO( &readable ) ;

#pragma warning(push, 3)
#pragma warning(disable: 4127)
    FD_SET( s_listenSocket , &readable ) ;
#pragma warning(default: 4127)
#pragma warning(pop)

      // Check for data waiting in s_listenSocket
    int returnSelect = select( 0 , &readable , NULL , NULL , &timeout ) ;
    while ( returnSelect != 0 && returnSelect != SOCKET_ERROR
           && FD_ISSET( s_listenSocket , &readable ) )
    {
      //-- Read Data --------------------------------------------------------//

          // Gather info
        const int          kBufferSize = 256 ;
        char               buffer[ kBufferSize ] ;  // filled by recvFrom call
        struct sockaddr_in address ;                // filled by recvFrom call
        int                sizeofAddr = sizeof ( struct sockaddr ) ;
        int returnRecv = recvfrom( s_listenSocket , buffer , kBufferSize , 0 ,
                                   reinterpret_cast< struct sockaddr * >( &address ) ,
                                   &sizeofAddr ) ;
        if ( returnRecv == SOCKET_ERROR )
            return ;  // run away~~!

          // Ensure string is null terminated
        buffer[ kBufferSize - 1 ] = '\0' ;
        if ( returnRecv < kBufferSize )
            buffer[ returnRecv ] = '\0' ;

		  // Convert for simpler syntax
		nsl::bstream stream ;
		stream.raw_set( reinterpret_cast< const nsl::byte_t * >( buffer ) , returnRecv ) ;

		  // Read and confirm Corner Hooked ID
		std::string chID ;
		stream >> chID ;
		if ( chID != kCornerHookedID )
			return ;  // some random packet from elsewhere hit our socket, perhaps

		  // Read and confirm packet id
		char id ;
		stream >> id ;
		if ( scast< int >( id ) != PacketGameDiscovery::ID )
			return ;  // some random packet from elsewhere hit or socket, perhaps

		  // Read game name
		std::string gameName ;
		stream >> gameName ;
		ASSERT( !gameName.empty() ) ;

		  // Read game type
		int gameType ;
		stream >> gameType ;

		  // Read host player name
		std::string hostName ;
		stream >> hostName ;

		  // Read player counts
		int numHumanPlayers, numAIPlayers, numAvailPlayers ;
		stream >> numHumanPlayers >> numAIPlayers >> numAvailPlayers ;

          // Flesh out NetGameInfo and the corresponding map entry
        NetGameInfo newGame ;
        newGame.address = inet_ntoa( address.sin_addr ) ;
        newGame.port = address.sin_port ;
        newGame.name = gameName ;
        newGame.gameType = scast< eGameType >( gameType ) ;
        newGame.hostPlayerName = hostName ;
        newGame.numHumanPlayers = numHumanPlayers ;
        newGame.numAIPlayers = numAIPlayers ;
        newGame.numAvailPlayers = numAvailPlayers ;
		newGame.timeSinceUpdate = 0 ;
        NetGamePair newMapValue ;
        newMapValue.first = newGame.address ;  // Key
        newMapValue.second = newGame ;         // Data

      //-- Update Game List -------------------------------------------------//

          // Look to see if it already exists
        bool found = false ;
        for ( NetGameItr itr = NetGamesBegin() ; itr != NetGamesEnd() ; ++itr )
        {
            if ( itr->first == newMapValue.first )
            {
                  // game info from this packet already exists
                found = true ;

                  // Let's update it in case the non-ip/port values changed
                itr->second.name = newMapValue.second.name ;
                itr->second.gameType = newMapValue.second.gameType ;
                itr->second.hostPlayerName = newMapValue.second.hostPlayerName ;
                itr->second.numHumanPlayers = newMapValue.second.numHumanPlayers ;
                itr->second.numAIPlayers = newMapValue.second.numAIPlayers ;
                itr->second.numAvailPlayers = newMapValue.second.numAvailPlayers ;

				  // Reset time elapsed since last update
				itr->second.timeSinceUpdate = 0 ;

                break ;
            }
        }
        if ( !found )
            s_games.insert( newMapValue ) ;  // Game is new; add it to list

      //---------------------------------------------------------------------//

        returnSelect = select( 0 , &readable , NULL , NULL , &timeout ) ;
    }

	  // Update elapsed times since last update and cull dead games
	NetGameItr itr = NetGamesBegin() ;
	while ( itr != NetGamesEnd() )
	{
		itr->second.timeSinceUpdate++ ;
		if ( itr->second.timeSinceUpdate > kCullingDelay )
		{
			NetGameItr tempItr = itr ;
			++itr ;
			s_games.erase( tempItr ) ;
		}
		else
		{
			++itr ;
		}
	}
//    for ( NetGameItr updateItr = NetGamesBegin() ; netItr != NetGamesEnd() ; ++netItr )
//		netItr->second.timeSinceUpdate++ ;
//	bool somethingDeleted = true ;
//	while ( somethingDeleted )
//	{
//		somethingDeleted = false ;
//        for ( NetGameItr itr = NetGamesBegin() ; itr != NetGamesEnd() ; ++itr )
//	      {
//			if ( itr->second.timeSinceUpdate > kCullingDelay )
//			{
//				s_games.erase( itr ) ;
//				somethingDeleted = true ;
//				break ;
//			}
//		}
//	}

    if ( returnSelect == SOCKET_ERROR )
        return ;  // run away~~!
    /* else no more data to be read */
}

/*****************************************************************************/
//-- NetGameRegister --------------------------------------------------------//
/*!
 *  \brief              Begin server advertisment/broadcasting
 */

void NetGameRegister( void )
{
      // Set broadcast delay to an immediate broadcast
    s_bcDelay = 0 ;

      // Set up destination addy
    struct sockaddr_in addr ;
    memset( &addr , 0 , sizeof ( struct sockaddr_in ) ) ;
    addr.sin_family = AF_INET ;
    addr.sin_port = htons( kDiscoveryServicePort ) ;
    addr.sin_addr.S_un.S_addr = htonl( INADDR_BROADCAST ) ;
    memcpy( &s_bcAddr , &addr , sizeof ( struct sockaddr ) ) ;

  //-- Set up UDP/Broadcast socket ------------------------------------------//

      // Initial socket setup
    s_bcSocket = socket( PF_INET , SOCK_DGRAM , 0 ) ;
    if ( s_bcSocket == INVALID_SOCKET )
        return ;  // run away~~!

      // Enable broadcasting in the socket
    bool isBroadcast = true ;
    if ( SOCKET_ERROR == setsockopt( s_bcSocket , SOL_SOCKET , SO_BROADCAST ,
                                     reinterpret_cast< char * >( &isBroadcast ) ,
                                     sizeof( isBroadcast ) ) )
    {
        return ;  // run away~~!
    }
}

/*****************************************************************************/
//-- NetGameUnregister ------------------------------------------------------//
/*! 
 *  \brief    End server advertisement/braodcasting
 */

void NetGameUnregister( void )
{
    closesocket( s_bcSocket ) ;
}

/*****************************************************************************/
//-- NetGameRegUpdate -------------------------------------------------------//
/*! 
 *  \brief    Broadcast if it might be necessary
 *
 *  \param    p_gameInfo    Information on this game
 */

void NetGameRegUpdate( const NetGameInfo & p_gameInfo )
{
    if ( --s_bcDelay <= 0 )
    {
          // reset the delay until another broadcast
        s_bcDelay = kBroadcastDelay ;

		  // construct packet
		nsl::bstream packet ;
		packet << kCornerHookedID ;
		packet << static_cast< char >( PacketGameDiscovery::ID ) ;
		packet << p_gameInfo.name ;
		packet << scast< int >( p_gameInfo.gameType ) ;
		packet << p_gameInfo.hostPlayerName ;
		packet << p_gameInfo.numHumanPlayers ;
		packet << p_gameInfo.numAIPlayers ;
		packet << p_gameInfo.numAvailPlayers ;
		ASSERT( packet.size() <= 256 ) ;

          // attempt a broadcast
        int returnValue = sendto( s_bcSocket ,
                                  reinterpret_cast< const char * >( packet.data() ) ,
                                  packet.size() ,
                                  0,
                                  static_cast< const struct sockaddr * >( &s_bcAddr ) ,
                                  sizeof( s_bcAddr ) ) ;
        if ( returnValue == SOCKET_ERROR || returnValue != static_cast<int>(packet.size()) )
            return ;  // run away~~!
    }
}

/*****************************************************************************/
//-- NetGamesBegin ----------------------------------------------------------//
/*!
 *  \brief     Recover an iterator to the first known game.
 *  \return    A constant interator to the first discovered game.
 */

NetGameItr NetGamesBegin( void )
{
  return ( s_games.begin() ) ;
}

/*****************************************************************************/
//-- NetGamesEnd ------------------------------------------------------------//
/*! 
 *  \brief     Recover an iterator to one beyond the last known game.
 *  \return    A constant interator to one beyond the last discovered game.
 */

NetGameItr NetGamesEnd( void )
{
  return ( s_games.end() ) ;
}

/*****************************************************************************/
//-- NetGamesSize -----------------------------------------------------------//
/*! 
 *  \brief     Returns the number of entries in the NetGames map
 *  \return    A number of entries
 */

NetGameMap::size_type NetGamesSize( void )
{
    return s_games.size() ;
}







/*****************************************************************************/
//-- Obsolete Code ----------------------------------------------------------//

/* --- BEGIN OLD InitDiscoveryInterface CODE ----------------------------------
    DNSServiceErrorType  err = kDNSServiceErr_NoError;

      // The cool thing about Rendezvous is that we only need to do this once,
      // and our callback will be updated whenever services be available or
      // unavailable. This is nice, because I'm lazy.
    err = DNSServiceBrowse(&gBrowserRef,0,0,kDiscoveryServiceName,0,NetGameServiceBrowseReply,0);
    if(err == kDNSServiceErr_NoError)
    {
          // Get the socket descriptor.
        int  sock =  DNSServiceRefSockFD(gBrowserRef);
 
          // Wait for reading asynchronously.
          // If there are no games available, we'll never get to read from this socket.
        ENFORCE(SOCKET_ERROR != ::WSAAsyncSelect(sock,Game::Get()->GetWindow()->GetHandle(),GM_NETDISC_BROWSEREPLY,FD_READ))
          ("Could not set up async read on discovery socket.");
    }
------ END OLD CODE -------------------------------------------------------- */

/* --- BEGIN OLD KillDiscoveryInterface CODE ----------------------------------
      // Kill the references.
    DNSServiceRefDeallocate(gBrowserRef);

    std::map< SOCKET,DNSServiceRef >::iterator  it;

      // Make sure to get all those still being used to resolve, as well.
    for(it = gResolveRefs.begin(); it != gResolveRefs.end(); ++it)
        DNSServiceRefDeallocate(it->second);
 
      // Release all game info.
    gGames.clear();
------ END OLD CODE -------------------------------------------------------- */

/* --- BEGIN OLD NetGameRegister CODE -----------------------------------------
    ASSERT(!gBroadcasting);
 
    DNSServiceErrorType  err = kDNSServiceErr_NoError;
    char                *txt;
    int                  len = NetGameBuildTXT(&txt,name);

    err = DNSServiceRegister(&gBroadcastRef,0,0,0,
                             kDiscoveryServiceName,0,0,
                             htons(kNetGamePort),
                             len,txt,
                             NetGameServiceBroadcastReply,0);
    delete[] txt;
    if(err == kDNSServiceErr_NoError)
    {
          // Get the socket descriptor.
        int  sock =  DNSServiceRefSockFD(gBrowserRef);
 
          // Wait for reading asynchronously.
        ENFORCE(SOCKET_ERROR != ::WSAAsyncSelect(sock,Game::Get()->GetWindow()->GetHandle(),GM_NETDISC_BROADCASTREPLY,FD_READ))
          ("Could not set up async read on discovery socket.");
        gBroadcasting = true;
    }
------ END OLD CODE -------------------------------------------------------- */

/*  ________________________________________________________________________ */
void NetGameHandleDiscoveryResponse(void)
/*! Handle a discovery response.

    This function is invoked from the WinSock asynchronous read handler.
*/
{
// // DNSServiceProcessResult(gBrowserRef);
}

/*  ________________________________________________________________________ */
void NetGameHandleBroadcastResponse(void)
/*! Handle a discovery response.

    This function is invoked from the WinSock asynchronous read handler.
*/
{
//  //DNSServiceProcessResult(gBroadcastRef);
}

/*  ________________________________________________________________________ */
void NetGameHandleResolveResponse(SOCKET /*sock*/)
/*! Handle a resolve response.

    This function is invoked from the WinSock asynchronous read handler.
    
    @param sock  The socket the resolution is occuring on.
*/
{
////DNSServiceRef  ref;
////std::map< SOCKET,DNSServiceRef >::iterator  it = gResolveRefs.find(sock);
////
////  if(it != gResolveRefs.end())
////  {
////    ref = it->second;
////    DNSServiceProcessResult(ref);
////    DNSServiceRefDeallocate(ref);
////    gResolveRefs.erase(it);    
////  }
} 

/*  ________________________________________________________________________ */
void NetGameHandleLookupResponse(int /*error*/,HANDLE /*hand*/)
/*! Handle a host name lookup response.

    This function is invoked from a WinSock asynchronous message handler.

    @param error  Error code from the message handler.
    @param hand   Task handle for the host name lookup.
*/
{
////HNLookupTaskMap::iterator  it = gHNLookupTasks.find(hand);
////
////  if(it != gHNLookupTasks.end())
////  {
////    // If an error occured, cancel the task.
////    if(error != 0)
////    {
////      WSACancelAsyncRequest(hand);
////      gHNLookupTasks.erase(it);
////    }
////    else
////    {
////    std::string  fullname      = it->second.first;
////    char        *buffer        = it->second.second;
////    hostent     *ent           = reinterpret_cast< hostent* >(buffer);
////    NetGameMap::iterator  game = gGames.find(fullname);
////    
////      if(game != gGames.end())
////      {
////        // Otherwise, the buffer we stored with the task will be filled out,
////        // so read the IP address off into permanent storage.
////        game->second.address = inet_ntoa(*(reinterpret_cast< in_addr*>(ent->h_addr)));
////      }
////      
////      // Clean up.
////      delete[] buffer;
////      gHNLookupTasks.erase(it);
////    }
////  }
}

///*  ________________________________________________________________________ */
//void CALLBACK NetGameDiscoveryUpdate(HWND /*wind*/,UINT /*msg*/,UINT_PTR /*tid*/,DWORD /*time*/)
///*! Update the discovery interface.
//
//    This function updates the client's list of available LAN games and,
//    if neccessary, the information about the client's own game.
//*/
//{
//}
//
///*  ________________________________________________________________________ */
////void DNSSD_API NetGameServiceBrowseReply(DNSServiceRef /*sdRef*/,DNSServiceFlags flags,uint32_t interfaceIndex,DNSServiceErrorType errorCode,const char *serviceName,const char *replyType,const char *replyDomain,void * /*context*/)
///*! Service browse reply callback.
//
//    If errorCode is nonzero, all other parameters are invalid.
//    
//    @param sdRef           The DNSServiceRef initialized by DNSServiceBrowse().
//    @param flags           Possible values are kDNSServiceFlagsMoreComing and kDNSServiceFlagsAdd.
//    @param interfaceIndex  The interface on which the service is advertised.
//    @param errorCode       0 on sucess, otherwise an error code.
//    @param serviceName     The service name discovered.
//    @param regtype         The service type.
//    @param domain          The domain on which the service was discovered.
//    @param context         The context pointer that was passed to the DNSServiceBrowse(). Should be null.
//*/
//{
//  //if(0 == errorCode)
//  //{
//  //  if(flags & kDNSServiceFlagsAdd)
//  //  {
//  //    // The service is newly-discovered, so add it.
//  //  NetGameInfo        info;
//  //  std::stringstream  fmt;
//  //  
//  //    fmt << serviceName << "." << replyType << replyDomain;
//  //    info.service = fmt.str();
//  //    info.name    = "";
//
//  //    gGames.insert(std::make_pair(fmt.str(),info));
//  //    
//  //    // Now try and resolve it to get detailed information.
//  //  DNSServiceRef        ref;
//  //  DNSServiceErrorType  err = kDNSServiceErr_NoError;
//  //  int                  sock = SOCKET_ERROR;
//  // 
//  //    err  = DNSServiceResolve(&ref,0,interfaceIndex,serviceName,replyType,replyDomain,NetGameServiceResolveReply,0);
//  //    sock = DNSServiceRefSockFD(ref);
//  //    if(err == kDNSServiceErr_NoError)
//  //    {
//  //      ENFORCE(SOCKET_ERROR != ::WSAAsyncSelect(sock,Game::Get()->GetWindow()->GetHandle(),GM_NETDISC_RESOLVEREPLY,FD_READ))
//  //             ("Could not set up async read on resolver socket.");
//  //      gResolveRefs.insert(std::make_pair(sock,ref));
//  //    }
//  //  }
//  //  else
//  //  {
//  //    // The service has gone away.
//  //  NetGameMap::iterator  it = gGames.find(serviceName);
//  //  
//  //    if(it != gGames.end())
//  //      gGames.erase(it);
//  //  }
//  //}
//}
//
///*  ________________________________________________________________________ */
//void DNSSD_API NetGameServiceBroadcastReply(DNSServiceRef sdRef,DNSServiceFlags flags,DNSServiceErrorType errorCode,const char *name,const char *regtype,const char *domain,void *context)
///*! Service broadcast reply callback.
//
//    If errorCode is nonzero, all other parameters are invalid.
//    
//    @param sdRef           The DNSServiceRef initialized by DNSServiceBrowse().
//    @param flags           Possible values are kDNSServiceFlagsMoreComing and kDNSServiceFlagsAdd.
//*/
//{
//}
//
///*  ________________________________________________________________________ */
//void DNSSD_API NetGameServiceResolveReply(DNSServiceRef sdRef,DNSServiceFlags flags,uint32_t interfaceIndex,DNSServiceErrorType errorCode,const char *fullname,const char *hosttarget,uint16_t port,uint16_t txtLen,const char *txtRecord,void *context)
///*! Service resolve reply callback.
//
//    If errorCode is nonzero, all other parameters are invalid.
//    
//    @param sdRef  The DNSServiceRef initialized by DNSServiceResolve().
//*/
//{
//  //if(0 == errorCode)
//  //{
//  //NetGameItr  it = gGames.find(fullname);
//  //
//  //  if(it != gGames.end())
//  //  {
//  //  HANDLE        taskHandle;
//  //  HNLookupTask  task;
//  //  char         *buffer = new char[MAXGETHOSTSTRUCT];
//
//  //    // We keep the address blank until we've resolved the IP address.
//  //    it->second.address = "";
//  //    it->second.port    = port;
//  //    
//  //    // Start a host name look up. When this completes we'll fill out the IP address.
//  //    taskHandle = WSAAsyncGetHostByName(Game::Get()->GetWindow()->GetHandle(),GM_NETDISC_LOOKUPDONE,hosttarget,buffer,MAXGETHOSTSTRUCT);
//  //    task = std::make_pair(fullname,buffer);
//  //    gHNLookupTasks.insert(std::make_pair(taskHandle,task));
//  //    
//  //    // Parse the TXT record.
//  //    NetGameParseTXT(txtRecord,it->second.name);
//  //  }
//  //}
//}
//
///*  ________________________________________________________________________ */
//int NetGameBuildTXT(char **txt,const std::string &name)
///*! Build a TXT record for a Corner Hooked game.
//    
//    The caller must delete[] the built TXT record.
//*/
//{
////size_t  sz = 0;
////
//// ASSERT(name.length() <= 255);
////
////  if(0 == txt)
////    return (0);
////  
////  // Compute TXT record size.
////  sz = name.length() + 1;
////  
////  // Build the record.
////  *txt = new char[sz];
////  
////  *txt[0] = static_cast< char >(name.length());
////  memcpy(*txt + 1,name.c_str(),name.length());
////  
////  return (sz);
//}
//
///*  ________________________________________________________________________ */
//void NetGameParseTXT(const char *txt,std::string &name)
///*! Parse a TXT record for a Corner Hooked game.
//*/
//{
////size_t  sz = *txt;
////char    *buf;
////
////  buf = new char[sz + 1];
////  memcpy(buf,txt + 1,sz);
////  name = buf;
//}