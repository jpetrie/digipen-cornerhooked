/*! ========================================================================

      @file    NetPackets.h
      @author  jmp
      @brief   Interface to network packet formats.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================  */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _NET_PACKETS_H_
#define _NET_PACKETS_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "nsl_bstream.h"

/*                                                                 constants
---------------------------------------------------------------------------- */

// max packet size
const int kNetPacketSz = 512;


/*                                                                   structs
---------------------------------------------------------------------------- */

struct PacketJoin
//! Join game packet.
{
  enum { ID = 1 };
  
  std::string  playerName;
};

struct PacketGameOptions
//! Packet containing game options.
{
  enum { ID = 2 };
  
  struct PlayerInfo
  {
    int          type;
    std::string  name;
  };
  
  std::string                 gameName;
  int                         playerCur;
  int                         playerMax;
  std::vector< PlayerInfo >   players;
  int                         gameType;
};

struct PacketGameStart
//! Packet signalling the start of the game.
{
  enum { ID = 3 };
  
  unsigned int  turn;
};

struct PacketTurn
//! Packet containing turn data.
{
  enum { ID = 4 };
  
  float  directionX;
  float  directionY;
  float  directionZ;
  float  power;
};

struct PacketEndTurnSync
//! Packet containing end-of-turn synchronization data.
{
  enum { ID = 5 };
  
  int  ball_count;
  std::vector< D3DXVECTOR3 >  balls;
  std::vector< char >         pocket_flags;
};

struct PacketChat
//! Packet containing chat message data.
{
  enum { ID = 6 };
  
  std::string  message;
};

struct PacketKick
//! Packet containing kick data.
{
  enum { ID = 7 };
  
  int  slot;  //!< Slot to kick a player out of.
};

//! \todo remove this forward declaration once rule system is checked in
enum eGameType ;

struct PacketGameDiscovery
//! Packet containing the necessary info for game discovery to work
{
	enum { ID = 8 } ;

	std::string m_chID ; // const game ID (since it is udp broadcasting rather than tcp connections)
	// (id=8 goes here in the packet structure)
	std::string m_gameName ;
	eGameType   m_gameType ;
	std::string m_hostPlayerName ;
	int         m_numHumanPlayers ;
	int         m_numAIPlayers ;
	int         m_numAvailPlayers ;
} ;

struct PacketCueAdjust
//! Packet containing cue adjustment data, used
//! during ball-in-hand.
{
  enum { ID = 9 };
  
  float  dx;
  float  dy;
  float  dz;
};

struct PacketQuit
//! Used to indicate a normal player or the host is quitting
{
	enum { ID = 10 };

	unsigned int slot ;  // Only used when a client sends a quit message
};

#endif  /* _NET_PACKETS_H_ */