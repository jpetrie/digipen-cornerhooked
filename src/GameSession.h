/*! ========================================================================

      @file    GameSession.h
      @author  jmp
      @brief   Interface to GameSession class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _GAME_SESSION_H_
#define _GAME_SESSION_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "StateMachine.h"

#include "NetServer.h"
#include "NetClient.h"

#include "Player.h"
#include "rulesystem.h"
#include "UIElement.h"
#include "UIPanel.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// player limit
const int  kPlayersMax = 8;

// player types
const int  kPlayerType_Avail  = 0; 
const int  kPlayerType_Closed = 1; 
const int  kPlayerType_Human  = 2; 
const int  kPlayerType_AI     = 3; 

// tutor bits
const int  kTBBallInHand   = 0;
const int  kTBShotLineup   = 1;
const int  kTBPowerResolve = 2;
const int  kTBWatchShot    = 3;
const int  kTBBitCount     = 4;


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class GameSession : public StateMachine
/*! A game session represents the state of an actual gameplay session.
*/
{
  // friends
  friend void SessionState_ShotLineupEnter(StateMachine *sm,float elapsed);
  friend void SessionState_ShotLineupExit(StateMachine *sm,float elapsed);
  friend void SessionState_ShotLineupUpdate(StateMachine *sm,float elapsed);
  friend void SessionState_BallInHandEnter(StateMachine *sm,float elapsed);
  friend void SessionState_BallInHandExit(StateMachine *sm,float elapsed);
  friend void SessionState_BallInHandUpdate(StateMachine *sm,float elapsed);
  friend void SessionState_PowerResolveEnter(StateMachine *sm,float elapsed);
  friend void SessionState_PowerResolveExit(StateMachine *sm,float elapsed);
  friend void SessionState_PowerResolveUpdate(StateMachine *sm,float elapsed);
  friend void SessionState_WatchShotEnter(StateMachine *sm,float elapsed);
  friend void SessionState_WatchShotExit(StateMachine *sm,float elapsed);
  friend void SessionState_WatchShotUpdate(StateMachine *sm,float elapsed);
  friend void SessionState_CallShotEnter(StateMachine *sm,float elapsed);
  friend void SessionState_CallShotExit(StateMachine *sm,float elapsed);
  friend void SessionState_CallShotUpdate(StateMachine *sm,float elapsed);
  
  friend bool Generic_KeyDownInt(int key);
  friend void ShotLineup_KeyDownInt(int key);
  friend void BallInHand_KeyDownInt(int key);
  friend void PowerResolve_KeyDownInt(int key);
  friend void WatchShot_KeyDownInt(int key);
  friend void CallShot_KeyDownInt(int key);
  
  friend bool Generic_LeftClickInt(int x,int y,int z);
  friend void ShotLineup_LeftClickInt(int x,int y,int z);
  friend void PowerResolve_LeftClickInt(int x,int y,int z);
  friend void WatchShot_LeftClickInt(int x,int y,int z);
  friend void BallInHand_LeftClickInt(int x,int y,int z);
  friend void CallShot_LeftClickInt(int x,int y,int z);
  
  friend bool Generic_RightClickInt(int x,int y,int z);
  friend void PowerResolve_RightClickInt(int x,int y,int z);
  friend void BallInHand_RightClickInt(int x,int y,int z);
  friend void CallShot_RightClickInt(int x,int y,int z);
  
  public:
    // ct and dt
    GameSession(bool host,const std::string &server = "",short port = 0);
    ~GameSession(void);
    
    // network update
    void UpdateGameOptions(const PacketGameOptions &pack);
    
    // accessors
    bool        IsHost(void) const        { return (mIsHost); }
    bool        IsPlaying(void) const     { return (mIsPlaying); }
    bool        IsMenuOpen(void) const    { return (mMenuPanel != 0); }
    
    // game name
    std::string GetGameName(void) const           { return (mName); }
    void        SetGameName(const std::string &n) { mName = n; }
    
    // players
    int     GetPlayersCur(void) const  { return (mPlayersCur); }
    int     GetPlayersMax(void) const  { return (kPlayersMax); }
    Player* GetPlayer(int i)           { ASSERT(i < kPlayersMax); return (mPlayers[i]); };
    void    SetPlayer(int i,Player *p) { ASSERT(i < kPlayersMax); mPlayers[i] = p; }
    
    // menu
    void     ShowMenu(const std::string &menu);
    void     HideMenu(void);
    UIPanel* GetMenu(void);
    
    // turns
    int  CurrentTurn(void) const { return (mCurrentPlayer); }
    void SetCurrentTurn(int p)     { mCurrentPlayer = p; }
    void AdvanceTurn(void);
    
    // gameplay/state control
    void HandleStart(void);
    void HandleShot(float vx,float vy,float vz,float power);
    void HandleChat(const std::string &msg);
    void HandleCueAdjust(float dx,float dy,float dz);
    
    // manipulators
    void SetName(const std::string &n) { mName = n; }
    void SetTutor(int bit);
    
    // state IDs
    unsigned int ShotLineupSID(void) const   { return (mShotLineupSID); }
    unsigned int BallInHandSID(void) const   { return (mBallInHandSID); }
    unsigned int PowerResolveSID(void) const { return (mPowerResolveSID); }
    unsigned int WatchShotSID(void) const    { return (mWatchShotSID); }
    unsigned int CallShotSID(void) const     { return (mCallShotSID); }
    
    Rules*  GetRules(void)     { /*ASSERT(0 != mRules);*/ return (mRules); }
    
    float   ballInHandPlaneDistance;
    
 // private:
    // typedefs
    typedef std::vector< Player* >  PlayerList;  //!< List of players in the game.
    
    // data members
    int  mShotLineupSID;    // State ID for shot lineup.
    int  mBallInHandSID;    // State ID for ball in hand.
    int  mPowerResolveSID;  // State ID for power resolution.
    int  mWatchShotSID;     // State ID for shot viewing.
    int  mCallShotSID;      // State ID for calling a shot.
    
    std::string  mName;        //!< Name of the game.
    int          mPlayersCur;  //!< Players currently in the game.
    int          mPlayersMax;  //!< Players allowed in the game, total.
    PlayerList   mPlayers;     //!< List of player names.
    
    UIPanel *mMenuPanel;  //!< Holds active menu.
    
    bool           mIsHost;  //!< If true, this session is the host for the game.
    NetServerData  mServer;  //!< Server data.
    NetClientData  mClient;  //!< Client data.
  
    int   mCurrentPlayer;  //!< Player ID of the active player.
    bool  mIsPlaying;  //!< If true, game has started.
        
    bool         mCamLocked;     //!< If true, mouse motion does not move camera.
    bool         mShotLocked;    //!< If true, camera motion does not affect shot vector.
    bool         mTutorVisible;  //!< If true, the tutor window is visible.
    D3DXVECTOR3  mLockedPos;     //!< Locked camera position when shot is fixed.
    
    bool         mTutorBits[kTBBitCount];     //!< If a bit is true, that section needs tutor display (first time).
    
    float  mMouseZoomDamp;
    float  mMouseSpeed;
    
    Rules  *mRules;		   //!< A rule system for governing shots.
};


#endif  /* _GAME_SESSION_H_ */