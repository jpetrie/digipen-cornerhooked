/*! ========================================================================

     @file    Game.h
     @author  jmc
     @brief   Interface to core game object.
     
     (c) 2004 DigiPen (USA) Corporation, all rights reserved.

    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _GAME_H_
#define _GAME_H_


/*                                                                 includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Window.h"
#include "Camera.h"
#include "Input.h"
#include "Clock.h"

#include "PlayfieldBase.h"
#include "StateMachine.h"
#include "GameSession.h"
#include "Skybox.h"

#include "UIScreen.h"
#include "UIElement.h"
#include "UIEditText.h"

#include "nsl_singleton.h"
#include "particles.h"
#include "soundengine.h"
#include "particle.h"

/*                                                                 constants
---------------------------------------------------------------------------- */

// ui positioning constants
const int  kUI_DistFromEdge = 5;  //!< UI element generic edge distance.

const int  kUI_TitleW  = 256;
const int  kUI_TitleH  = 256;
const int  kUI_TransSz = 75;

const int  kUI_MMButtonW = 128;  //!< Main menu button width.
const int  kUI_MMButtonH = 32;   //!< Main menu button height.  

const int  kUI_MPanelW = 640;    //!< Width of setup panels.
const int  kUI_MPanelH = 480;    //!< Height of setup panels.
const int  kUI_GPanelW = 160;    //!< Width of in-game menu panel.
const int  kUI_GSPanelH = 265;   //!< Height of Game Select Screen panel is smaller.

const int  kUI_GPanelButtonH = 48;  //!< Height of in-game panel buttons.

const int  kUI_EditBoxH  = 20;
const int  kUI_RadioBtnH = 20;
const int  kUI_RadioBtnW = 20;

const int  kUI_GSListW       = 200;
const int  kUI_GSListH       = 200;
const int  kUI_GSInfoW       = 300;
const int  kUI_GSInfoH       = 200;
const int  kUI_GSPlayerNameW = 200;
const int  kUI_GSGameListX   = 40;

const int  kUI_GOPlayersListX = 16;
const int  kUI_GOPlayersListW = 150;
const int  kUI_GOPlayersNumW  = 16;
const int  kUI_GOPlayersKickW = 32;
const int  kUI_GOPlayersAddW  = 20;
const int  kUI_GOTypeX        = 300;
const int  kUI_GOTypeY        = 200;
const int  kUI_GOTypeW        = 200;

const int  kUI_GPChatBoxW       = 360;
const int  kUI_GPStatusBarW     = 300;
const int  kUI_GPStatusBarH     = 32;
const int  kUI_GPResultsPanelW  = 400;
const int  kUI_GPScoreboardW    = 300;
const int  kUI_GPJukeboxW       = 600;
const int  kUI_GPJBPlaylistH    = 300;
const int  kUI_GPIndW           = 64;
const int  kUI_GPIndH           = 64;
const int  kUI_GPTurnIndicatorW = 400;
const int  kUI_GPTurnIndicatorH = 30;
const int  kUI_GPTutorW         = kUI_GPChatBoxW;
const int  kUI_GPTutorH         = 200;

// ui transition constants
const float  kUI_TitleStall      = 0.0f;
const float  kUI_TitleSpeed      = 1.0f;

const float  kUI_PlayBtnStall    = 0.0f;
const float  kUI_PlayBtnSpeed    = 1.0f;
const float  kUI_OptionsBtnStall = 0.0f;
const float  kUI_OptionsBtnSpeed = 1.0f;
const float  kUI_CreditsBtnStall = 0.0f;
const float  kUI_CreditsBtnSpeed = 1.0f;
const float  kUI_QuitBtnStall    = 0.0f;
const float  kUI_QuitBtnSpeed    = 1.0f;

const float  kUI_MPanelStall     = 0.0f;
const float  kUI_MPanelSpeed     = 1.0f;

// ui names strings
const std::string  kUI_Title1Str      = "data/misc/title01.png";    //!< "Cor"
const std::string  kUI_Title2Str      = "data/misc/title02.png";    //!< "ner"
const std::string  kUI_Title3Str      = "data/misc/title03.png";    //!< "Hoo"
const std::string  kUI_Title4Str      = "data/misc/title04.png";    //!< "ked"
const std::string  kUI_Opts1Str       = "data/misc/options01.png";  
const std::string  kUI_Opts2Str       = "data/misc/options02.png";
const std::string  kUI_Credits1Str    = "data/misc/credits01.png";
const std::string  kUI_Credits2Str    = "data/misc/credits02.png";
const std::string  kUI_GS1Str         = "data/misc/select01.png";
const std::string  kUI_GS2Str         = "data/misc/select02.png";
const std::string  kUI_GS3Str         = "data/misc/select03.png";
const std::string  kUI_GO1Str         = "data/misc/setup01.png";
const std::string  kUI_GO2Str         = "data/misc/setup02.png";
const std::string  kUI_GO3Str         = "data/misc/setup03.png";
const std::string  kUI_GPInHandStr    = "data/misc/ballinhand.png";
const std::string  kUI_GPLineupStr    = "data/misc/shotlineup.png";
const std::string  kUI_GPSetPowerStr  = "data/misc/setpower.png";
const std::string  kUI_GPCallShotStr  = "data/misc/callshot.png";
const std::string  kUI_GPCamIndStr    = "data/misc/cam.png";
const std::string  kUI_GPCueIndStr    = "data/misc/cue.png";
const std::string  kUI_GPLockIndStr   = "data/misc/lock.png";
const std::string  kUI_RadioBtnOnStr  = "data/misc/radioon.png";
const std::string  kUI_RadioBtnOffStr = "data/misc/radiooff.png";

const std::string  kUI_PlayBtnCap     = "Play Game";
const std::string  kUI_OptionsBtnCap  = "Options";
const std::string  kUI_CreditsBtnCap  = "Credits";
const std::string  kUI_QuitBtnCap     = "Quit";

const std::string  kUI_BackBtnCap      = "Go Back";
const std::string  kUI_GSJoinBtnCap    = "Join Game";
const std::string  kUI_GSCreateBtnCap  = "Create Game";
const std::string  kUI_GSGameHelpText  = "Select a game to see information on it, or to join it.\n"
                                         "Or, use the Create Game button to start a new game.";
const std::string  kUI_GSGameNameText  = "(no game selected)";
const std::string  kUI_GSGameTypeCap   = "Game Type:  ";
const std::string  kUI_GSGameHostCap   = "Host Player:  ";
const std::string  kUI_GSGameHumanPCap = "Human players in this game:  ";
const std::string  kUI_GSGameAIPCap    = "AI players in this game:  ";
const std::string  kUI_GSGameAvailCapS = "This game is looking for ";
const std::string  kUI_GSGameAvailCapE = " more players.";
const std::string  kUI_GSGameAvailFullCap = "!!  This game does not have any available slots.  !!";

const std::string  kUI_GOStartBtnCap      = "Start Game";
const std::string  kUI_GOSlotAvailBtnCap  = "<available>";
const std::string  kUI_GOSlotCloseBtnCap  = "<closed>";
const std::string  kUI_GOSlotAIBtnCap     = "<AI Player>";
const std::string  kUI_GOKickBtnCap       = "Kick";
const std::string  kUI_GOCloseBtnCap      = "Close";
const std::string  kUI_GOOpenBtnCap       = "Open";
const std::string  kUI_GOAddAIBtnCap      = "AI";
const std::string  kUI_GOChatSendBtnCap   = "Send Message";
const std::string  kUI_GOGameTypeBtnCap[ GAME_TYPE_COUNT ] = {
	                                          "Eighteen Ball",
                                              "Nineteen Ball"/*,
                                              "Straight Pool",
                                              "Two-Eleven"*/ };

const std::string  kUI_GPScoreboardBtnCap   = "S\nc\no\nr\ne\nb\no\na\nr\nd" ;
const std::string  kUI_GPJukeboxBtnCap      = "J\nu\nk\ne\nb\no\nx" ;
const std::string  kUI_GPHideVertBtnCap     = ">\n \nh\ni\nd\ne\n \n p\na\nn\ne\nl\n \n>" ;
const std::string  kUI_GPYourTurnText       = "It is now your turn.";
const std::string  kUI_GPTheirTurnTextS     = "It is now player ";
const std::string  kUI_GPTheirTurnTextM     = " (";
const std::string  kUI_GPTheirTurnTextE     = ")'s turn.";
const std::string  kUI_GPResultsWinCap      = "You are the winner!";
const std::string  kUI_GPResultsLoseCap     = "You have lost!";
const std::string  kUI_GPResultsPlayerCap[ GAME_TYPE_COUNT ] = {
	                                          " has sunk the 18-ball.",
                                              " has sunk the 19-ball."/*,
                                              " is the first to reach 150 points.",
                                              " is the first to achieve their private number."*/ };

const std::string  kUI_OptsSaveBtnCap     = "Apply";
const std::string  kUI_OptsBackBtnCap     = "Back";
const std::string  kUI_OptsVideoTabCap    = "Video";
const std::string  kUI_OptsAudioTabCap    = "Audio";
const std::string  kUI_OptsControlsTabCap = "Controls";
const std::string  kUI_OptsGameTabCap     = "Gameplay";


// internal element name strings
const std::string  kUI_GSJoinButton    = "JoinButton";
const std::string  kUI_GSPanelName     = "SelectPanel";
const std::string  kUI_GSListName      = "GameList";
const std::string  kUI_GSInfoName      = "GameInfo";
const std::string  kUI_GSPlayerName    = "PlayerName";
const std::string  kUI_GSSelGameName   = "SelGame - name";
const std::string  kUI_GSSelGameType   = "SelGame - gameType";
const std::string  kUI_GSSelGameHost   = "SelGame - hostPlayerName";
const std::string  kUI_GSSelGameHumanP = "SelGame - numHumanPlayers";
const std::string  kUI_GSSelGameAIP    = "SelGame - numAIPlayers";
const std::string  kUI_GSSelGameAvailP = "SelGame - numAvailPlayers";

const std::string  kUI_GOPanelName      = "SetupPanel";
const std::string  kUI_GOTitleName      = "TitleText";
const std::string  kUI_GOPButtonName[] = { "P0Button",
                                           "P1Button",
                                           "P2Button",
                                           "P3Button",
                                           "P4Button",
                                           "P5Button",
                                           "P6Button",
                                           "P7Button" };
const std::string  kUI_GOPKickButtonName[] = { "P0KickButton",
                                               "P1KickButton",
                                               "P2KickButton",
                                               "P3KickButton",
                                               "P4KickButton",
                                               "P5KickButton",
                                               "P6KickButton",
                                               "P7KickButton" };
const std::string  kUI_GOPAddAIButtonName[] = { "P0AddButton",
                                                "P1AddButton",
                                                "P2AddButton",
                                                "P3AddButton",
                                                "P4AddButton",
                                                "P5AddButton",
                                                "P6AddButton",
                                                "P7AddButton" };
const std::string  kUI_GOTypeMenuName       = "GametypeMenu";
const std::string  kUI_GOGameTypeBtnName[ GAME_TYPE_COUNT ] = {
	                                            "18-Ball Button",
                                                "19-Ball Button"/*,
                                                "StraightPool Button",
                                                "8-11 Button"*/ };
const std::string  kUI_GOGameTypeImgName[ GAME_TYPE_COUNT ] = {
	                                            "18-Ball Image",
                                                "19-Ball Image"/*,
                                                "StraightPool Image",
                                                "8-11 Image"*/ } ;
const std::string  kUI_GOGameTypePnlName[ GAME_TYPE_COUNT ] = {
	                                            "18-Ball Panel",
                                                "19-Ball Panel"/*,
                                                "StraightPool Panel",
                                                "8-11 Panel"*/ } ;
const std::string  kUI_GOGameTypeDscName[ GAME_TYPE_COUNT ] = {
	                                            "18-Ball Description",
                                                "19-Ball Description"/*,
                                                "StraightPool Description",
                                                "8-11 Description"*/ } ;
const std::string  kUI_GOChatDisplayName    = "ChatDisplay";
const std::string  kUI_GOChatEntryName      = "ChatEntry";
const std::string  kUI_GOChatSendButtonName = "ChatSend";

const std::string  kUI_GPTutorName            = "SpankyTheMuskrat";
const std::string  kUI_GPPowerMeterName       = "PowerMeter";
const std::string  kUI_GPChatBoxName          = "ChatBox";
const std::string  kUI_GPStatusPanelName      = "StatusPanel";
const std::string  kUI_GPStatusTextName       = "StatusText";
const std::string  kUI_GPHelpPanelName        = "HelpPanel";    //!< A menu.
const std::string  kUI_GPMenuPanelName        = "MenuPanel";    //!< A menu.
const std::string  kUI_GPResultsPanelName     = "ResultsPanel";
const std::string  kUI_GPResultsWinLoseName   = "ResultsWinOrLose";
const std::string  kUI_GPResultsWinnerName    = "WinnerPlayerName";
const std::string  kUI_GPScoreboardToggleName = "SBToggle";
const std::string  kUI_GPJukeboxToggleName    = "JBToggle";
const std::string  kUI_GPScoreboardPanelName  = "SBPanel";
const std::string  kUI_GPSBTitleName          = "SBTitle";
const std::string  kUI_GPSBPlayerTextName[]   = { "P0 SBText" ,
                                                  "P1 SBText" ,
                                                  "P2 SBText" ,
                                                  "P3 SBText" ,
                                                  "P4 SBText" ,
                                                  "P5 SBText" ,
                                                  "P6 SBText" ,
                                                  "P7 SBText" };
const std::string  kUI_GPSBScoreTextName[]   = { "P0 Score" ,
                                                 "P1 Score" ,
                                                 "P2 Score" ,
                                                 "P3 Score" ,
                                                 "P4 Score" ,
                                                 "P5 Score" ,
                                                 "P6 Score" ,
                                                 "P7 Score" };
const std::string  kUI_GPJukeboxPanelName     = "JBPanel";
const std::string  kUI_GPJBTitleName          = "JBTitle";
const std::string  kUI_GPJBCurrentSongName    = "JBCurSong";
const std::string  kUI_GPJBCurSongDetailsName = "JBCurSongDetails";
const std::string  kUI_GPJBPlaylistName       = "JBPlaylist";
const std::string  kUI_GPJBPlayButtonName     = "JBPlay";
const std::string  kUI_GPJBStopButtonName     = "JBStop";
const std::string  kUI_GPJBPrevButtonName     = "JBPrevious";
const std::string  kUI_GPJBNextButtonName     = "JBNext";
const std::string  kUI_GPMsgBallInHandName    = "MsgBallInHand";
const std::string  kUI_GPMsgShotLineupName    = "MsgShotLineup";
const std::string  kUI_GPMsgSetPowerName      = "MsgSetPower";
const std::string  kUI_GPMsgCallShotName      = "MsgCallShot";
const std::string  kUI_GPCamIndName           = "IndCam";
const std::string  kUI_GPCamLockName          = "LockCam";
const std::string  kUI_GPCueIndName           = "IndCue";
const std::string  kUI_GPCueLockName          = "LockCue";
const std::string  kUI_GPTurnIndicatorName    = "TurnIndicator";
const std::string  kUI_GPTurnIndTextName      = "TurnIndicatorText";

const std::string  kUI_OptsVideoPanelName   = "OptsVideoPanel";
const std::string  kUI_OptsVideoResListName = "ResolutionList";

const std::string  kUI_OptsAudioPanelName = "OptsAudioPanel";

const std::string  kUI_OptsControlsPanelName = "OptsControlsPanel";

const std::string  kUI_OptsGamePanelName = "OptsGamePanel";

const std::string  kUI_CreditsPanelName   = "CreditsPanel";
const std::string  kUI_CreditsTextName    = "CreditsText";
const std::string  kUI_CreditsBackBtnName = "BackButton";

// message area constants
const int    kUI_MsgAreaX         = kUI_DistFromEdge;
const int    kUI_MsgAreaY         = 2 * kUI_DistFromEdge + kUI_GPStatusBarH + 25;
const int    kUI_MsgAreaW         = 256;
const int    kUI_MsgAreaH         = 256;
const int    kMsgAreaLimit        = 8;     //!< Message limit.
const float  kMsgAreaLifetime     = 4.0f;  //!< Default message lifetime (seconds).
const int    kMsgAreaAdjThreshold = 100;   //!< Under this many chars, don't adjust lifetime.
const int    kMsgAreaCharPerAdj   = 10;    //!< This many characters...
const float  kMsgAreaTimePerAdj   = 0.2f;  //!< ...add this many seconds to the lifetime.

// camera movement constants
const float kCam_Speed = 15.0f;

/*                                                                   structs
---------------------------------------------------------------------------- */

struct GlideAnimData
//! Encapsulates glide animation data.
{
  D3DXVECTOR3  eyeStart;     //!< Camera pan start position.
  D3DXVECTOR3  eyeStop;      //!< Camera pan stop position.
  D3DXVECTOR3  eyeP0;        //!< Camera pan intermediate vector 1.
  D3DXVECTOR3  eyeP1;        //!< Camera pan intermediate vector 2.
  D3DXVECTOR3  targetStart;  //!< Target pan start position.
  D3DXVECTOR3  targetStop;   //!< Target pan stop position.
  D3DXVECTOR3  targetP0;     //!< Target pan intermediate vector 1.
  D3DXVECTOR3  targetP1;     //!< Target pan intermediate vector 2.
  float        time;         //!< Pan interpolation time.
};

struct ShotAnimData
//! Encapsulates shot animation data.
{
  D3DXVECTOR3  vec;       //!< Shot vector (unit).
  float        power;     //!< Shot power (velocity).
  bool         taken;     //!< True once shot impulse has been applied.
  
  D3DXVECTOR3  cueTip;       //!< Position of tip of cue.
  D3DXVECTOR3  cueButt;      //!< Position of butt of cue.
  float        cueTime;      //!< Elapsed time for cue animation.
  int          cueStrokes;   //!< Number of "test strokes" taken.
  float        engAngle;     //!< English angle.
  float        endDistance;  //!< English distance (from center).
};


/*                                                                   classes
---------------------------------------------------------------------------- */

// forward declarations
class Player;
class AIPlayer;
class Rules;
class ShotProject;

/*  ________________________________________________________________________ */
class Game : public StateMachine, public nsl::singleton< Game >
/*! Encapsulates game components and functionality.
*/
{
  public:
    // ct and dt
    Game(void);
    ~Game(void);
    
    // init
    void Init(void);
    
    // playloop
	  void Playloop(void);
	  
	  // accessors
	  bool DrawFPS(void) const      { return (mDrawFPS);  }
	  bool DrawCue(void) const      { return (mDrawCue);  }
    bool DrawGhost(void)const     { return (mDrawGhost);}
    bool DrawMessages(void)const  { return (mDrawMsg);  }
    bool DrawLitBalls(void) const { return (mDrawLitBalls); } 
    
     
	  //D3DXVECTOR3 vu,vr; // what are these for?
	  bool        needToSpot;

      // manipulators
      void DrawMessages(bool f) { mDrawMsg   = f; }
	  void DrawFPS(bool f)         { mDrawFPS = f; }
	  void DrawCue(bool f)         { mDrawCue = f; }
	  void DrawBallNumbers(bool f) { mDrawBallNum = f; } 
	  void DrawGhost(bool f)       { mDrawGhost = f; }
	  void DrawLitBalls(bool f)    { mDrawLitBalls = f; }
	
    // components
    Window*                 GetWindow(void)    { ASSERT(0 != mWindow); return (mWindow); }
    Camera*                 GetCamera(void)    { ASSERT(0 != mCamera); return (mCamera); }
    Input*                  GetInput(void)     { ASSERT(0 != mInput); return (mInput); }
    UIScreen*               GetScreen(void)    { ASSERT(0 != mScreen); return (mScreen); }
	UIScreen*               GetScreen(const std::string &screen) { ASSERT(0 != mScreens[screen]); return (mScreens[screen]); }
    Playfield*              GetPlayfield(void) { ASSERT(0 != mPlayfield); return (mPlayfield); }
    Physics::Engine*        GetPhysics(void)   { ASSERT(0 != mPhysics); return (mPhysics); }
    Physics::Engine*        GetAIPhysics(void) { ASSERT(0 != mAIPhysics); return (mAIPhysics); }
    AIPlayer*               GetAIPlayer(void)  { ASSERT(0 != mAIPlayer); return (mAIPlayer); }
    GameSession*            GetSession(void)   { /*ASSERT(0 != mSession);*/ return (mSession); }
    SoundManager*           GetSound(void)     { ASSERT(0 != mSound); return (mSound); }
    ShotProject*            GetAimBall(void)   { ASSERT(0 != mAim);   return (mAim); }

  	ParticleSystem *GetParticles(void) { ASSERT(0 != mParticleSystem); return (mParticleSystem); }

	Tracker<unsigned int>	ti;
   // Particles< DXSphere, particle_rule, kParticleCount >* GetParticles(void)  { ASSERT(0 != mParticles); return (mParticles); }
	  
	  // session existence
	  bool DoesSessionExist(void) const { return (mSession != 0); }
	
	  // screen access
	  void SetActiveScreen(const std::string &screen);
	  void RemoveScreen(const std::string &screen);

    // session access
    void CreateSession(bool host,const std::string &server = "",short port = 0);
    void DestroySession(void);
    	
	  // state IDs
	  int GetMainMenuStateID(void) const    { return (mMainMenuStateID); }
	  int GetGameSelectStateID(void) const  { return (mGameSelectStateID); }
	  int GetGameOptionsStateID(void) const { return (mGameOptionsStateID); }
	  int GetGameplayStateID(void) const    { return (mGameplayStateID); }
	  int GetOptionsStateID(void) const     { return (mOptionsStateID); }
	  int GetCreditsStateID(void) const     { return (mCreditsStateID); }
	  
	  // messaging
	  void WriteMessage(const std::string &msg);
	  void UpdateMessages(void);
	  void PrintMessages(void);
	  void ClearMessages(void);
	  
	  // turn test
	  bool IsMyTurn(void) { return (mPlayerTurn == GetSession()->CurrentTurn()); }
	  
	  // player data
	  std::string GetMyName(void) const             { return (mPlayerName); }
	  void        SetMyName(const std::string &n)   { mPlayerName = n; }
	  int         GetMyTurn(void) const             { return (mPlayerTurn); }
	  void        SetMyTurn(int t)                  { mPlayerTurn = t; }
	  std::string GetMyServer(void) const           { return (mPlayerHost); }
	  void        SetMyServer(const std::string &s) { mPlayerHost = s; }
	  short       GetMyPort(void) const             { return (mPlayerPort); }
	  void        SetMyPort(short p)                { mPlayerPort = p; }

	  // shot data
	  D3DXVECTOR3 GetMyShotVector(void) const     { return (mShotVector); }
    void        SetMyShotVector(D3DXVECTOR3 &v) { mShotVector = v; }
 
	  D3DXVECTOR2 GetMyShotOffset(void) const     { return (mShotOffset); }
	  void        SetMyShotOffset(D3DXVECTOR2 &v) { mShotOffset = v; }
	  
	  // animation data
	  ShotAnimData&   GetShotAnimData(void)  { return (mShotAnimData); }
    GlideAnimData&  GetGlideAnimData(void) { return (mGlideAnimData); }

    std::string  GetTutorString(int i) { return (mTutorStrings[i]); }
    std::string  mTutorStrings[4];  //!< 
    int          lookingAtBall;     //!< 0 means nothing, anything else, ball number.

	  int  splash1SID;
	  int  splash2SID;



	  // Jukebox accessors
	  
	  
	struct JBInfo
	{
		int         m_songID ;        //!< Returned from SoundManager::Load2DObject
		std::string m_songFilename ;  //!< Loaded from ::GetJukeboxSongs
	} ;
	signed int             GetCurrentJBIndex( void )                { return m_curPlayingIndex ; }
	void                   SetCurrentJBIndex( signed int p_songID ) { m_curPlayingIndex = p_songID ; }
	signed int             GetCurrentJBSong( void )                 { return m_curPlayingID ; }
	void                   SetCurrentJBSong( signed int p_songID )  { m_curPlayingID = p_songID ; }
	std::vector< JBInfo >& GetJBSongs( void )                       { return m_jukeboxSongs ; }

	  // Game End / Results update/timer
	void UpdateResultPanel( void ) ;

  private:
    // ui screen builders
    void nInitMMUI(void);
    void nInitGSUI(void);
  public:
    void nInitGOUI(void);
    void nInitGPUI(void);
  private:
    
    void nInitOptionsGameUI(void);
    void nInitOptionsVideoUI(void);
    void nInitOptionsSoundUI(void);
    void nInitOptionsControlsUI(void);
	void nInitCreditsUI(void);
    
    // message lifetime adjuster
    float nAdjustMsgLifetime(const std::string &msg);
    
    // data members
    Window      *mWindow;   //!< Main window object.
    GameSession *mSession;  //!< Active game session.
    
    UIScreen                          *mScreen;   //!< Points to the active screen.
	  std::map< std::string,UIScreen* >  mScreens;  //!< All UI screens.
	  
	  // The following fields are storage for game state IDs.
	  int  mMainMenuStateID;		//!< ID of main menu state.
	  int  mGameSelectStateID;	//!< ID of game selection state.
	  int  mGameOptionsStateID;	//!< ID of game setup state.
	  int  mGameplayStateID;		//!< ID of gameplay state.
	  int  mOptionsStateID;     //!< ID of options state.
	  int  mCreditsStateID;     //!< ID of credits state.


    Camera *mCamera;  //!< Camera.
    Input  *mInput;   //!< DirectInput manager.

    Skybox *mSkybox;
    Playfield						   *mPlayfield;  ///< Where the shit goes down.
    	  
	Physics::Engine        *mPhysics;	    ///< The physics engine;
	ParticleSystem		   *mParticleSystem;
    Physics::Engine        *mAIPhysics;     ///< The AI physics simulation	
    AIPlayer               *mAIPlayer;      ///< An AI player
    SoundManager           *mSound;         ///< Sound Engine
	ShotProject            *mAim;           ///< A projected ball for aiming
	  
	  DXFont *mFPSFont;  //!< For rendering the FPS display.
	  DXFont *mMsgFont;  //!< For rendering the message queue.
	  
	  Particles< DXSphere, particle_rule, kParticleCount >*	mParticles;
	  
	  // The following fields are relevant to the message printing
	  // interface.
	  std::vector< std::string >  mMsgHistory;   //!< All text messages.
	  unsigned int                mMsgStart;     //!< First message to draw.
	  Clock                       mMsgTimer;     //!< Message timer.
	  float                       mMsgLifetime;  //!< Message lifetime.
	  
	  // The following fields are storage for local player data and/or
	  // settings. They are accessed via the GetMy*/SetMy* methods and
	  // are valid only for the local machine.
	  std::string  mPlayerName;  //!< Local player name.
	  int          mPlayerTurn;  //!< Local player's turn ID.
	  std::string  mPlayerHost;  //!< Server name local machine is connecting to.
	  short        mPlayerPort;  //!< Server port local machine is connecting to.
	  D3DXVECTOR2  mShotOffset;  //!< Offset (degrees) of shot, from view.
	  D3DXVECTOR3  mShotVector;  //!< Vector along which shot will be taken.
	  
	  // The following fields are for animation or "resolution" purposes
	  // only. They are not trustworthy outside of the "watch shot" game
	  // session state.
	  ShotAnimData   mShotAnimData;   //!< Data for animating shots.
	  GlideAnimData  mGlideAnimData;  //!< Data for animating camera glides.
	  
	  // The following fields are toggles for various animation effects.
	  bool  mDrawFPS;
	  bool  mDrawCue;
    bool  mDrawGhost;
    bool  mDrawMsg;
    bool  mDrawBallNum;
    bool  mDrawLitBalls;

	public:
      // Turn Indicator update/timer
	void UpdateTurnIndicator( void ) ;
	private:

	  // Jukebox data
	std::vector< JBInfo > m_jukeboxSongs ;   //!< Songs available to jukebox
	signed int            m_curPlayingIndex; //!< Index of currently playing song (in vector/listbox)
	signed int            m_curPlayingID ;   //!< ID of currently playing song (from fmod)
};

/*                                                                prototypes
---------------------------------------------------------------------------- */

// windows events
LRESULT CALLBACK Callback_WM_SYSCOMMAND(Window *wind,UINT msg,WPARAM wp,LPARAM lp);

LRESULT CALLBACK Callback_GM_NETDISC_BROWSEREPLY(Window *wind,UINT msg,WPARAM wp,LPARAM lp);
LRESULT CALLBACK Callback_GM_NETDISC_BROADCASTREPLY(Window *wind,UINT msg,WPARAM wp,LPARAM lp);
LRESULT CALLBACK Callback_GM_NETDISC_RESOLVEREPLY(Window *wind,UINT msg,WPARAM wp,LPARAM lp);
LRESULT CALLBACK Callback_GM_NETDISC_LOOKUPDONE(Window *wind,UINT msg,WPARAM wp,LPARAM lp);

LRESULT CALLBACK Callback_GM_NETSERVER_ACCEPT(Window *wind,UINT msg,WPARAM wp,LPARAM lp);
LRESULT CALLBACK Callback_GM_NETSERVER_READ(Window *wind,UINT msg,WPARAM wp,LPARAM lp);
LRESULT CALLBACK Callback_GM_NETCLIENT_READ(Window *wind,UINT msg,WPARAM wp,LPARAM lp);

// exit cleanup handler
void ExitFinished(void);

// game state - main menu
void GameState_Splash1Enter(StateMachine *sm,float elapsed);
void GameState_Splash1Exit(StateMachine *sm,float elapsed);
void GameState_Splash1Update(StateMachine *sm,float elapsed);

// game state - main menu
void GameState_Splash2Enter(StateMachine *sm,float elapsed);
void GameState_Splash2Exit(StateMachine *sm,float elapsed);
void GameState_Splash2Update(StateMachine *sm,float elapsed);

// game state - main menu
void GameState_MainMenuEnter(StateMachine *sm,float elapsed);
void GameState_MainMenuExit(StateMachine *sm,float elapsed);
void GameState_MainMenuUpdate(StateMachine *sm,float elapsed);

// game state - game selection
void GameState_GameSelectEnter(StateMachine *sm,float elapsed);
void GameState_GameSelectExit(StateMachine *sm,float elapsed);
void GameState_GameSelectUpdate(StateMachine *sm,float elapsed);

// game state - game setup
void GameState_GameOptionsEnter(StateMachine *sm,float elapsed);
void GameState_GameOptionsExit(StateMachine *sm,float elapsed);
void GameState_GameOptionsUpdate(StateMachine *sm,float elapsed);

// game state - gameplay
void GameState_GameplayEnter(StateMachine *sm,float elapsed);
void GameState_GameplayExit(StateMachine *sm,float elapsed);
void GameState_GameplayUpdate(StateMachine *sm,float elapsed);

// game state - options
void GameState_OptionsEnter(StateMachine *sm,float elapsed);
void GameState_OptionsExit(StateMachine *sm,float elapsed);
void GameState_OptionsUpdate(StateMachine *sm,float elapsed);

// game state - credits
void GameState_CreditsEnter(StateMachine *sm,float elapsed);
void GameState_CreditsExit(StateMachine *sm,float elapsed);
void GameState_CreditsUpdate(StateMachine *sm,float elapsed);

// key handlers
void KeyHandler_MainMenu(int key);

// mouse handlers
void LCHandler_MainMenu(int x,int y,int z);

// UI events - main menu
int UI_MMPlayClick(void);
int UI_MMOptionsClick(void);
int UI_MMCreditsClick(void);
int UI_MMQuitClick(void);

// UI events - game selection
int UI_GSBackClick(void);
int UI_GSCreateGameClick(void);
int UI_GSJoinGameClick(void);

// UI events - game setup
int UI_GOBackClick(void);
int UI_GOStartClick(void);
int UI_GONameLostFocus(void);
int UI_GOChatLostFocus(void);
int UI_GOP0ButtonKickClick(void);
int UI_GOP1ButtonKickClick(void);
int UI_GOP2ButtonKickClick(void);
int UI_GOP3ButtonKickClick(void);
int UI_GOP4ButtonKickClick(void);
int UI_GOP5ButtonKickClick(void);
int UI_GOP6ButtonKickClick(void);
int UI_GOP7ButtonKickClick(void);
int UI_GOPButtonKickClick(int which);
int UI_GOP0ButtonAddAIClick(void);
int UI_GOP1ButtonAddAIClick(void);
int UI_GOP2ButtonAddAIClick(void);
int UI_GOP3ButtonAddAIClick(void);
int UI_GOP4ButtonAddAIClick(void);
int UI_GOP5ButtonAddAIClick(void);
int UI_GOP6ButtonAddAIClick(void);
int UI_GOP7ButtonAddAIClick(void);
int UI_GOPButtonAddAIClick(int which);
int UI_GOSendChatClick(void);

// Game Type Radio Button stuff
int UI_GOGameType1Click(void);
int UI_GOGameType2Click(void);
//int UI_GOGameType3Click(void);
//int UI_GOGameType4Click(void);
const UIElement::Callback UI_GOGameTypeClick[ GAME_TYPE_COUNT ] = {
	                              UI_GOGameType1Click , UI_GOGameType2Click /*,
	                              UI_GOGameType3Click , UI_GOGameType4Click*/ } ;
extern int gCurrentGameType ;

// UI events - gameplay
int UI_GPEnterGameplay(void);
int UI_GPMenuResumeClick(void);
int UI_GPMenuResignClick(void);
int UI_GPMenuQuitClick(void);
int UI_GPSBToggleClick(void);
int UI_GPJBToggleClick(void);
int UI_GPJBPlayClick(void);
int UI_GPJBStopClick(void);
int UI_GPJBPrevClick(void);
int UI_GPJBNextClick(void);

// UI events - options
int UI_OptsSaveClick(void);
int UI_OptsBackClick(void);

// physics callbacks
void Physics_OnPlaneContactCB(Collision::Contact* c,Physics::RigidBody *p,Physics::RigidBody *s);
void Physics_OnPocketContactCB(Collision::Contact* c,Physics::RigidBody *p,Physics::RigidBody *s);
void Physics_OnSphereContactCB(Collision::Contact*, Physics::RigidBody*, Physics::RigidBody*);
void Physics_OnStaticCB(Collision::Contact* c,Physics::RigidBody *p,Physics::RigidBody *s);

//@todo HACKS
int UI_DontClick(void);

void SplashKeyInt(int key);
void SplashClickInt(int x,int y,int z);

#endif  /* _GAME_H_ */