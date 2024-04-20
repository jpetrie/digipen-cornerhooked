/*! ========================================================================

      @file    NetGameDiscovery.h
      @author  jmp
      @brief   Interface to game discovery.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _NET_GAMEDISCOVERY_H_
#define _NET_GAMEDISCOVERY_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// discovery update timer
const int  kNetGameDiscovery_Timer   = 7737;
const int  kNetGameDiscovery_Timeout = 1000;  // Milliseconds between updates.


/*                                                                   structs
---------------------------------------------------------------------------- */

//! \todo remove this forward declaration once rule system is checked in
enum eGameType ;

struct NetGameInfo
//! Describes what a client wants to know when making a decision to join or not
{
    std::string  address;  //!< Host address.
    short        port;     //!< Host port.
 
    std::string  name ;                //!< Name of the game.
	eGameType    gameType ;            //!< Game rule type
	std::string  hostPlayerName ;      //!< Name of the host (player, not server)
	int          numHumanPlayers ;     //!< Number of slots filled by human players
	int          numAIPlayers ;        //!< Number of slots filled by AI players
	int          numAvailPlayers ;     //!< Number of available slots (ie total - closed - human - AI)

	unsigned int timeSinceUpdate ;     //!< Wait since the last time this was updated
};


/*                                                                  typedefs
---------------------------------------------------------------------------- */

typedef std::map< std::string , NetGameInfo >  NetGameMap ;
typedef std::pair< std::string , NetGameInfo > NetGamePair ; // string non-const
typedef NetGameMap::iterator                   NetGameItr ;


/*                                                                prototypes
---------------------------------------------------------------------------- */ 

// setup / shutdown (client -- build game list)
void InitDiscoveryInterface(void);
void KillDiscoveryInterface(void);
void UpdateDiscoveryInterface(void);

// game list
NetGameItr NetGamesBegin(void);
NetGameItr NetGamesEnd(void);
NetGameMap::size_type NetGamesSize(void);

// registration (server -- advertise game)
void NetGameRegister(void);
void NetGameUnregister(void);
void NetGameRegUpdate( const NetGameInfo & p_gameInfo ) ;

#endif  /* _NET_GAMEDISCOVERY_H_ */