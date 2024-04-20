/*! ========================================================================

      @file    GameSession.cpp
      @author  jmp
      @brief   Implementation of GameSession class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "GameSession.h"

#include "Input.h"

#include "NetGameDiscovery.h"

#include "UIScreen.h"
#include "UIElement.h"
#include "UIGraphic.h"
#include "UIButton.h"
#include "UIPowerMeter.h"
#include "PhysicsAux.h"
#include "AIPlayer.h"
#include "RuleSystem.h"
#include "shotprojection.h"

#include "nsl_random.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const D3DXVECTOR3  gBIHLookPoint(0.0f,0.0f,-25.0f);
  const D3DXVECTOR3  gBIHBackPoint(0.0f,50.0f,-100.0f);
  const D3DXVECTOR3  gBIHSidePoint(-100.0f,0.0f,-25.0f);
}


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  float  gBIHAnimTime = 0.0f;
  float  gBIHAnimStep = 1.0f;
  bool   gBIHAnimate  = false;
  
  random  gRNG(129045);
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
GameSession::GameSession(bool host,const std::string &server,short /*port*/)
/*! Constructor.

    @param host  If true, this session is the game host.
*/
: mMenuPanel(0),
  mIsHost(host),mCamLocked(false),mShotLocked(false),
  mName("Balls to the Wall"),///@todo something more professional maybe
  mPlayersCur(0),mPlayersMax(0),
  mCurrentPlayer(0),
  mIsPlaying(false),
  mRules(0)
{
  //Game::Get()->WriteMessage("CT GAMESESSION");
  // Create states.
  mShotLineupSID   = DefineState(SessionState_ShotLineupUpdate,SessionState_ShotLineupEnter,SessionState_ShotLineupExit);
  mBallInHandSID   = DefineState(SessionState_BallInHandUpdate,SessionState_BallInHandEnter,SessionState_BallInHandExit);
  mPowerResolveSID = DefineState(SessionState_PowerResolveUpdate,SessionState_PowerResolveEnter,SessionState_PowerResolveExit);
  mWatchShotSID    = DefineState(SessionState_WatchShotUpdate,SessionState_WatchShotEnter,SessionState_WatchShotExit);
  mCallShotSID     = DefineState(SessionState_CallShotUpdate,SessionState_CallShotEnter,SessionState_CallShotExit);
  
  // Are we the host?
  if(mIsHost)
  {
    InitServer(&mServer);
    InitClient(&mClient,"127.0.01",kNetGamePort);
    NetGameRegister();
  }
  else
  {
    InitClient(&mClient,server,kNetGamePort);
  }
  
  // Fill the player list with null pointers, meaning
  // "nobody here yet, thanks."
  for(unsigned int i = 0; i < kPlayersMax; ++i)
    mPlayers.push_back(0);
  
  // Set all tutor bits.
  for(unsigned int i = 0; i < kTBBitCount; ++i)
    mTutorBits[i] = true;
  
  // Read config values.
char buffer[256];

  ::GetPrivateProfileString("Mouse","ZoomDamp","0.5",buffer,256,"data/config/user.ini");
  mMouseZoomDamp = scast<float>(atof(buffer));
  ::GetPrivateProfileString("Mouse","Speed","20.0",buffer,256,"data/config/user.ini");
  mMouseSpeed = scast<float>(atof(buffer));
  ballInHandPlaneDistance = -24.0f;
}

/*  ________________________________________________________________________ */
GameSession::~GameSession(void)
/*! Destructor.
*/
{
  //Game::Get()->WriteMessage("DT GAMESESSION");
  Game::Get()->SetMyServer( "" ) ;
  KillClient() ;
  if(mIsHost)
  {
    KillServer();
    NetGameUnregister();
  }
}

void GameSession::SetTutor(int bit)
{
	std::stringstream lbz;
	if(bit == 1)
	{
		lbz << "Legal balls:\n";
		for(unsigned int i = 0; i < Game::Get()->GetSession()->GetRules()->GetLegalBalls(Game::Get()->GetSession()->CurrentTurn()).size(); ++i)
		{
			lbz << Game::Get()->GetSession()->GetRules()->GetLegalBalls(Game::Get()->GetSession()->CurrentTurn())[i] << " ";
		}
	}
	static_cast< UIEditText* >(Game::Get()->GetScreen()->GetElement(kUI_GPTutorName))->SetText(Game::Get()->GetTutorString(bit) + lbz.str());
}

/*  ________________________________________________________________________ */
void GameSession::UpdateGameOptions(const PacketGameOptions &pack)
/*! Update the game session with new data from the server.

    @param p  Packet containing new game options.
*/
{
UIPanel  *p = static_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GOPanelName));
UIButton *b = 0;

  // Alter game name.
  SetGameName(pack.gameName);
  
  // Set current/max players.
  mPlayersCur = pack.playerCur;
  mPlayersMax = pack.playerMax;
  
  // Walk the player list...
  for(unsigned int i = 0; i < 8; ++i)
  {
    // If the local slot is empty, and the remote slot is
    // now full, insert the new player.
    if(mPlayers[i] == 0 && pack.players[i].type != kPlayerType_Avail && pack.players[i].type != kPlayerType_Closed)
    {
      Player *player = new Player();
    
      // Set the name.
      b = static_cast< UIButton* >(p->GetElement(kUI_GOPButtonName[i]));
      b->SetCaption(pack.players[i].name);
      player->SetName(pack.players[i].name);
      
      // Update kick/AI buttons accordingly.
      b = static_cast< UIButton* >(p->GetElement(kUI_GOPKickButtonName[i]));
      b->SetCaption(kUI_GOKickBtnCap);
      b = static_cast< UIButton* >(p->GetElement(kUI_GOPAddAIButtonName[i]));
      b->Enable(false);
       
      // Put the player in the list.
      Game::Get()->GetSession()->SetPlayer(i,player);
    }
    // If the local slot was full, and the remote slot
    // is empty, remove the player.
    else if(mPlayers[i] != 0 && (pack.players[i].type == kPlayerType_Avail || pack.players[i].type == kPlayerType_Closed))
    {
      if (pack.players[i].type == kPlayerType_Avail)
	  {
        // Set the name.
        b = static_cast< UIButton* >(p->GetElement(kUI_GOPButtonName[i]));
        b->SetCaption(kUI_GOSlotAvailBtnCap);
	  }
	  else
	  {
        // Set the name.
        b = static_cast< UIButton* >(p->GetElement(kUI_GOPButtonName[i]));
        b->SetCaption(kUI_GOSlotCloseBtnCap);
	  }
    }
  }
  if ( !IsHost() )
	gCurrentGameType = pack.gameType ;
}

/*  ________________________________________________________________________ */
void GameSession::ShowMenu(const std::string &menu)
/*! Show a menu screen.

    @param menu  The name of the menu panel element.
*/
{
  // Don't try two menus at once. It will make the first one jealous.
  ASSERT(mMenuPanel == 0);
  
  mMenuPanel = static_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(menu));
  
  // Sanity testing.
  ASSERT(mMenuPanel != 0);
  ASSERT(mMenuPanel->GetType() == UIElement::kPanel);
  
  // Show it.
  mMenuPanel->Die(false);
  mMenuPanel->Reset();
  
  // Show the cursor so people can click on stuff.
  if(menu == kUI_GPMenuPanelName)
    ::ShowCursor(true);
  
  // Hide panels.
UIButton *b = static_cast< UIButton* >(Game::Get()->GetScreen()->GetElement(kUI_GPJukeboxToggleName));

  if(b->GetCaption() == kUI_GPHideVertBtnCap)
    UI_GPJBToggleClick();
  b = static_cast< UIButton* >(Game::Get()->GetScreen()->GetElement(kUI_GPScoreboardToggleName));
  if(b->GetCaption() == kUI_GPHideVertBtnCap)
    UI_GPSBToggleClick();
}

/*  ________________________________________________________________________ */
void GameSession::HideMenu(void)
/*! Hide a menu screen.
    
    No effect if no menu is open.
*/
{
  if(mMenuPanel != 0)
  {
    mMenuPanel->Die(true);
    mMenuPanel->Exit();
    mMenuPanel = 0;
    
  CURSORINFO  ci = { 0 };
  
    ci.cbSize = sizeof(ci);
    ::GetCursorInfo(&ci);
    if(ci.flags & CURSOR_SHOWING)
      ::ShowCursor(false);
  }
}

/*  ________________________________________________________________________ */
UIPanel* GameSession::GetMenu(void)
/*! Get the menu panel.
    
    @return
    A pointer to the menu UIPanel, or null if no menu is open.
*/
{
  return (mMenuPanel);
}

/*  ________________________________________________________________________ */
void GameSession::AdvanceTurn(void)
/*! Advance to the next turn.
*/
{
int  turn = (CurrentTurn() + 1) % kPlayersMax;
std::stringstream fmt;

  // Advance until we have a non-null player.
  while(GetPlayer(turn) == 0)
    turn = (turn + 1) % kPlayersMax;
  mCurrentPlayer = turn;
  
  if(Game::Get()->GetSession()->CurrentTurn() == Game::Get()->GetMyTurn() && Game::Get()->needToSpot)
  {
      TransitionTo(Game::Get()->GetSession()->BallInHandSID());
      Game::Get()->needToSpot = false;
  }

    
	if ( !Game::Get()->GetSession()->GetRules()->GameOver() )
	{
		  // Display player turn indicator
		UIPanel * p = rcast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GPTurnIndicatorName ) ) ;
		p->Die( false ) ;
		p->Reset() ;

		  // Update player turn indicator text
		UIEditText * e = rcast< UIEditText * >( p->GetElement( kUI_GPTurnIndTextName ) ) ;
		if ( Game::Get()->IsMyTurn() )
		{
			e->SetText( kUI_GPYourTurnText ) ;
		}
		else
		{
			std::ostringstream turnMessage ;
			turnMessage << kUI_GPTheirTurnTextS << mCurrentPlayer << kUI_GPTheirTurnTextM
						<< GetPlayer( mCurrentPlayer )->GetName() << kUI_GPTheirTurnTextE ;
			e->SetText( turnMessage.str() ) ;
		}
	}
}

/*  ________________________________________________________________________ */
void GameSession::HandleStart(void)
/*! Handle the starting of gameplay.
    This function should get called from a network handler once the "start
    game" packet has been received and parsed.
*/
{
  // We're playing the game now.
  mIsPlaying = true;
  mCurrentPlayer = 0;

  if(gCurrentGameType == EIGHTEEN_BALL)
  {
	  Game::Get()->GetPlayfield()->LoadBalls(EIGHTEEN_BALL);
	  Game::Get()->GetPlayfield()->RackBalls(EIGHTEEN_BALL);
	  mRules = new Rules_EighteenBall();
  }
  else if(gCurrentGameType == NINETEEN_BALL)
  {
	mRules = new Rules_NineteenBall();
	Game::Get()->GetPlayfield()->LoadBalls(NINETEEN_BALL);
	Game::Get()->GetPlayfield()->RackBalls(NINETEEN_BALL);
  }
//  else if(gCurrentGameType == STRAIGHT_POOL)
//  {
//
//  }
//  else if(gCurrentGameType == TWO_ELEVEN)
//  {
//
//  }
  
  
  // Make sure the game play GUI is loaded up.
  UI_GPEnterGameplay();
}


/*  ________________________________________________________________________ */
void GameSession::HandleShot(float vx,float vy,float vz,float power)
/*! Handle the resolution of a shot.
    This function should get called from a network handler once a turn packet
    has been received and parsed. It will deal with actually causing the shot
    to occur, including camera transitions and applying impulse to the ball.
    
    @param vx     Z component of the shot vector.
    @param vy     Y component of the shot vector.
    @param vz     Z component of the shot vector.
    @param power  Power of the shot.
*/
{
  // Transition to the viewing state. Store shot data for eventual use
  // once the camera glide is complete.
  TransitionTo(mWatchShotSID);
  
  ///@todo @GEOMETRY ISSUE
Geometry::Vector3D  cbv = Game::Get()->GetPlayfield()->GetBallPosition(Game::Get()->GetPhysics(),0);

D3DXVECTOR3  camPos;
D3DXVECTOR3  newPos;
D3DXVECTOR3  camTarget;
D3DXVECTOR3  shotVec(vx,vy,vz);
D3DXVECTOR3  cueBall(cbv[0],cbv[1],cbv[2]);

  // We'll need these.
  Game::Get()->GetCamera()->GetPosVector(&camPos);
  Game::Get()->GetCamera()->GetTargetVector(&camTarget);
  //if(Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn())->IsAI())
  //{
  //}
  //else
  //{

  //}
  //
  // The shot vector should already be normalized for us.
  Game::Get()->GetShotAnimData().vec        = shotVec;
  Game::Get()->GetShotAnimData().power      = power;
  Game::Get()->GetShotAnimData().taken      = false;
  Game::Get()->GetShotAnimData().cueTip     = cueBall + (-shotVec * 10.0f);
  Game::Get()->GetShotAnimData().cueButt    = cueBall + (-shotVec * 60.0f);
  Game::Get()->GetShotAnimData().cueTime    = 0.0f;
  Game::Get()->GetShotAnimData().cueStrokes = 0;

  Game::Get()->GetGlideAnimData().eyeStart    = camPos;
  Game::Get()->GetGlideAnimData().eyeStop     = cueBall + (-shotVec * 60.0f);
  Game::Get()->GetGlideAnimData().targetStart = camTarget;
  Game::Get()->GetGlideAnimData().targetStop  = cueBall;
  Game::Get()->GetGlideAnimData().time        = 0.0f;	  
}

/*  ________________________________________________________________________ */
void GameSession::HandleChat(const std::string &msg)
/*! Handle an incoming chat broadcast.
    This function should get called from a network handler once a chat packet
    has been recieved and parsed.
*/
{
  // Pretty simple, here.
  Game::Get()->WriteMessage(msg);
}

/*  ________________________________________________________________________ */
void GameSession::HandleCueAdjust(float dx,float dy,float dz)
/*! Handle an incoming cue adjustment broadcast.

    @param dx  X adjustment delta.
    @param dy  Y adjustment delta.
    @param dz  Z adjustment delta.
*/
{
Geometry::Vector3D  cueBall(dx,dy,dz);
std::stringstream  fmt;

  fmt << dx << "," << dy << "," << dz;
  Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(),Physics::Engine::eRigidBodyVector::propPosition,cueBall);
  //Game::Get()->WriteMessage(fmt.str());
}

/*  ________________________________________________________________________ */
void SessionState_ShotLineupEnter(StateMachine *sm,float /*elapsed*/)
/*! State enter function for shot lineup.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
    //if(GetBallByNumber(0)->Pocketed())
    //{
    //    GetBallByNumber(0)->Pocketed(false);
        //Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(), Physics::Engine::eRigidBodyVector::propPosition, Vector3D(0,0,-25));
        //if(Game::Get()->GetSession()->CurrentTurn() == Game::Get()->GetMyTurn())
        //   sm->TransitionTo(Game::Get()->GetSession()->BallInHandSID());
    //}

    Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn())->SetBestShot(Game::Get()->GetAIPlayer()->SelectShot());
GameSession *session = static_cast< GameSession* >(sm);
Game        *game    = Game::Get();
Input       *input   = game->GetInput();

  Game::Get()->GetSession()->SetTutor(kTBShotLineup);

  // Install the input handlers.
  input->InstallKeyDownHandler(ShotLineup_KeyDownInt);
  input->InstallLeftClickHandler(ShotLineup_LeftClickInt);
  input->InstallRightClickHandler(0);
  
  // Look at the cue ball.  //@todo @GEOMETRY ISSUE
Geometry::Vector3D  target = game->GetPlayfield()->GetBallPosition(game->GetPhysics(),0);

  game->GetCamera()->SetTarget(target[0],target[1],target[2]);	

  // Unlock.
  session->mCamLocked = false;
  session->mShotLocked = false;

  // Show state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgShotLineupName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgShotLineupName)->Reset();

  // Hide the locks.
  Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Die(true);
  Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Exit();
  Game::Get()->GetScreen()->GetElement(kUI_GPCueLockName)->Die(true);
  Game::Get()->GetScreen()->GetElement(kUI_GPCueLockName)->Exit();

  // The following UI elements are dead on this screen:
  Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName)->Die(true);
}

/*  ________________________________________________________________________ */
void SessionState_ShotLineupExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for shot lineup.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  // Hide state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgShotLineupName)->Die(true);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgShotLineupName)->Exit();
}

/*  ________________________________________________________________________ */
void SessionState_ShotLineupUpdate(StateMachine *sm,float /*elapsed*/)
/*! State update function for shot lineup.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
GameSession *session = static_cast< GameSession* >(sm);
Game        *game    = Game::Get();

  // Move the camera.
  if(!session->mCamLocked && !session->IsMenuOpen())
  {
  D3DXVECTOR3  shotVector;
  D3DXVECTOR2  shotOffset = game->GetMyShotOffset();
  
    // values for zoom damping
    float range = (game->GetCamera()->GetMaxTargetDistance() - game->GetCamera()->GetMinTargetDistance()) + 1.f;
    float dist = game->GetCamera()->GetDistToTarget();
    float ratio = (dist / range) * session->mMouseZoomDamp;

    game->GetCamera()->OrbitX(game->GetInput()->MouseXDelta() / session->mMouseSpeed);
    game->GetCamera()->OrbitY(game->GetInput()->MouseYDelta() / session->mMouseSpeed);
    game->GetCamera()->TrackZ(game->GetInput()->MouseZDelta() * ratio);
    
    // Adjust the shot vector.
    if(!session->mShotLocked)
    {
    D3DXVECTOR3  right;
    D3DXVECTOR3  up;
    D3DXMATRIX   out;
    
      game->GetCamera()->GetViewVector(&shotVector);
      game->GetCamera()->GetUpVector(&up);
      D3DXVec3Normalize(&shotVector,&shotVector);
      D3DXVec3Normalize(&up,&up);
      D3DXVec3Cross(&right,&up,&shotVector);
    
      // Rotate shot vector around basis vectors.
      D3DXMatrixRotationAxis(&out,&up,-shotOffset.x);
      D3DXVec3TransformCoord(&shotVector,&shotVector,&out);
      D3DXVec3Normalize(&shotVector,&shotVector);
      D3DXMatrixRotationAxis(&out,&right,-shotOffset.y);
      D3DXVec3TransformCoord(&shotVector,&shotVector,&out);
      D3DXVec3Normalize(&shotVector,&shotVector);
  
      if(game->GetInput()->MouseXDelta() || game->GetInput()->MouseYDelta())
      {
        game->SetMyShotVector(shotVector);
        game->GetPlayfield()->mShotProj = game->GetAimBall()->Project();
      }
    }
  }
}

/*  ________________________________________________________________________ */
void SessionState_BallInHandEnter(StateMachine * /*sm*/,float /*elapsed*/)
/*! State enter function for ball-in-hand.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
//GameSession *session = static_cast< GameSession* >(sm);
Game        *game    = Game::Get();
Input       *input   = game->GetInput();

  Game::Get()->GetSession()->SetTutor(kTBBallInHand);

  // Install the input handlers.
  input->InstallKeyDownHandler(BallInHand_KeyDownInt);
  input->InstallLeftClickHandler(BallInHand_LeftClickInt);
  input->InstallRightClickHandler(BallInHand_RightClickInt);
  
  // Look somewhere useful.
  game->GetCamera()->SetTarget(gBIHLookPoint.x,gBIHLookPoint.y,gBIHLookPoint.z);	
  game->GetCamera()->SetPosition(gBIHBackPoint.x,gBIHBackPoint.y,gBIHBackPoint.z);
  
  // Show state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgBallInHandName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgBallInHandName)->Reset();

  // The following UI elements are dead on this screen:
  Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName)->Die(true);
}

/*  ________________________________________________________________________ */
void SessionState_BallInHandExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for ball in hand.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  // Hide state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgBallInHandName)->Die(true);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgBallInHandName)->Exit();
}

/*  ________________________________________________________________________ */
void SessionState_BallInHandUpdate(StateMachine * /*sm*/,float elapsed)
/*! State update function for ball in hand.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = Game::Get();

  // These hard-coded damp numbers are not meant to be in user.ini;
  // they should remain fixed at these values.
float  dx =  game->GetInput()->MouseXDelta() / 25.0f;
float  dy = -game->GetInput()->MouseYDelta() / 25.0f;
float  dz =  game->GetInput()->MouseZDelta() / 75.0f;
  
  if(game->GetSession()->mCamLocked || game->GetSession()->IsMenuOpen())
    return;

Geometry::Vector3D  cueBall = Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(),Physics::Engine::eRigidBodyVector::propPosition);
  
  // Constrain the movement of the ball.
  if(abs(cueBall[0] + dx) < (game->GetPlayfield()->mWidth / 2.0f - 1.0f))
    cueBall[0] += dx;
  if(abs(cueBall[1] + dy) < (game->GetPlayfield()->mHeight / 2.0f - 1.0f))
    cueBall[1] += dy;
  if(cueBall[2] + dz < game->GetSession()->ballInHandPlaneDistance && cueBall[2] + dz > -(game->GetPlayfield()->mDepth / 2.0f - 6.0f))
    cueBall[2] += dz;
  
  Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(),Physics::Engine::eRigidBodyVector::propPosition,cueBall);

  // Move camera so that the player gets a good view.
D3DXVECTOR3  outEye;
  
  if(gBIHAnimate)
  {
    // Stop animating if we've reached the destination.
    if(gBIHAnimTime > 1.0f || gBIHAnimTime < 0.0f)
      gBIHAnimate = false;
    else
    {
      D3DXVec3Lerp(&outEye,&gBIHBackPoint,&gBIHSidePoint,gBIHAnimTime);
      gBIHAnimTime += 4.0f * elapsed * gBIHAnimStep;
      game->GetCamera()->SetPosition(outEye.x,outEye.y,outEye.z);
    }
  }
}

/*  ________________________________________________________________________ */
void SessionState_PowerResolveEnter(StateMachine* /*sm*/,float /*elapsed*/)
/*! State enter function for power resolution.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Input *input = Game::Get()->GetInput();

  Game::Get()->GetSession()->SetTutor(kTBPowerResolve);

  // Install appropriate input handlers.
  input->InstallKeyDownHandler(PowerResolve_KeyDownInt); 
  input->InstallLeftClickHandler(PowerResolve_LeftClickInt);
  input->InstallRightClickHandler(PowerResolve_RightClickInt);
  
  // Show state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgSetPowerName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgSetPowerName)->Reset();
  
  // Show the locks.
  Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Reset();
  Game::Get()->GetScreen()->GetElement(kUI_GPCueLockName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPCueLockName)->Reset();
  
  // The following UI elements are alive on this screen:
  Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName)->Reset();
}

/*  ________________________________________________________________________ */
void SessionState_PowerResolveExit(StateMachine* /*sm*/,float /*elapsed*/)
/*! State cleanup function for power resolution.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  // Send UI elements home.
  Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName)->Exit();
  
  // Hide state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgSetPowerName)->Die(true);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgSetPowerName)->Exit();
}

/*  ________________________________________________________________________ */
void SessionState_PowerResolveUpdate(StateMachine * /*sm*/,float /*elapsed*/)
/*! State update function for power resolution.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
 if(Game::Get()->GetSession()->IsMenuOpen())
    return;

  // Track vertical mouse movement to update power meter.
float delta = Game::Get()->GetInput()->MouseYDelta() / 1000.0f;
  
UIPowerMeter *pm = static_cast< UIPowerMeter* >(Game::Get()->GetScreen()->GetElement("PowerMeter"));

  pm->SetPower(pm->GetPower() - delta); 
}

/*  ________________________________________________________________________ */
void SessionState_WatchShotEnter(StateMachine * /*sm*/,float /*elapsed*/)
/*! State enter function for shot viewing.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game  *game  = Game::Get();
Input *input = game->GetInput();

  Game::Get()->GetSession()->SetTutor(kTBWatchShot);

  // Instal input handlers.
  input->InstallKeyDownHandler(WatchShot_KeyDownInt);
  input->InstallLeftClickHandler(WatchShot_LeftClickInt);
  input->InstallRightClickHandler(0);

  // The following UI elements are dead on this screen:
  Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName)->Die(true);
}

/*  ________________________________________________________________________ */
void SessionState_WatchShotExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for shot viewing.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
}

/*  ________________________________________________________________________ */
void SessionState_WatchShotUpdate(StateMachine *sm,float elapsed)
/*! State update function for shot viewing.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
GameSession   *session = static_cast< GameSession* >(sm);
Game          *game    = Game::Get();
ShotAnimData  &shot    = game->GetShotAnimData();
GlideAnimData &glide   = game->GetGlideAnimData();
D3DXVECTOR3    outEye;
D3DXVECTOR3    outPos;

  // 
  D3DXVec3Lerp(&outEye,&glide.eyeStart,&glide.eyeStop,glide.time);
  D3DXVec3Lerp(&outPos,&glide.targetStart,&glide.targetStop,glide.time);
  glide.time += elapsed;
  if(glide.time > 1.0f && !shot.taken)
  {
    // Adjust the cue position.
  Geometry::Vector3D  cbv = game->GetPlayfield()->GetBallPosition(game->GetPhysics(),0);
  D3DXVECTOR3         cueBall(cbv[0],cbv[1],cbv[2]);
 
    shot.cueTime += elapsed;//0.025f;
    game->GetShotAnimData().cueTip  = cueBall + (-shot.vec * (10.0f + (10.0f * sin(shot.cueTime * 6.0f))));
    game->GetShotAnimData().cueButt = cueBall + (-shot.vec * (60.0f + (10.0f * sin(shot.cueTime * 6.0f))));

    if(shot.cueTime > 1.0f && !shot.taken)
    {
    Geometry::Vector3D  v(shot.vec.x,shot.vec.y,shot.vec.z);
    
      // The transition is complete. Now we actually take the shot
      // by applying impulse to the cue ball.
      game->GetPhysics()->Disturb();
      game->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[0]->ID(),Physics::Engine::propVeloctity,v * shot.power);
      if(Game::Get()->GetSession()->GetRules())
        game->GetSession()->GetRules()->TookShot(true);
      
      // Mark the shot as resolved, because Josh is an idiot.
      // This way we won't keep forcing the cue into the wall.
      shot.taken = true;
    }
  }
  else if(!shot.taken)
  {
    // Animate camera.
    game->GetCamera()->SetPosition(outEye.x,outEye.y,outEye.z);
    game->GetCamera()->SetTarget(outPos.x,outPos.y,outPos.z);
  }
  else if(shot.taken && game->GetPhysics()->AtRest() && session->CurrentTurn() == game->GetMyTurn())
  {
    // Local machine turn; go to shot lineup.
    session->TransitionTo(session->ShotLineupSID());
  }
  else if(shot.taken)
  {
    game->GetCamera()->OrbitX(game->GetInput()->MouseXDelta() / session->mMouseSpeed);
    game->GetCamera()->OrbitY(game->GetInput()->MouseYDelta() / session->mMouseSpeed);
    game->GetCamera()->TrackZ(game->GetInput()->MouseZDelta() / session->mMouseSpeed);
  }
}

/*  ________________________________________________________________________ */
void SessionState_CallShotEnter(StateMachine* /*sm*/,float /*elapsed*/)
/*! State enter function for call shot.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Input *input = Game::Get()->GetInput();

  Game::Get()->GetSession()->SetTutor(kTBPowerResolve);

  // Install appropriate input handlers.
  input->InstallKeyDownHandler(CallShot_KeyDownInt); 
  input->InstallLeftClickHandler(CallShot_LeftClickInt);
  input->InstallRightClickHandler(CallShot_RightClickInt);
  
  // Show state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgCallShotName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgCallShotName)->Reset();
}

/*  ________________________________________________________________________ */
void SessionState_CallShotExit(StateMachine* /*sm*/,float /*elapsed*/)
/*! State cleanup function for call shot.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  // Hide state name element.
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgCallShotName)->Die(true);
  Game::Get()->GetScreen()->GetElement(kUI_GPMsgCallShotName)->Exit();
}

/*  ________________________________________________________________________ */
void SessionState_CallShotUpdate(StateMachine * /*sm*/,float /*elapsed*/)
/*! State update function for call shot.

    @param sm       The state machine (convertable to GameSession).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
 if(Game::Get()->GetSession()->IsMenuOpen())
    return;
    
  // values for zoom damping
  float range = (Game::Get()->GetCamera()->GetMaxTargetDistance() - Game::Get()->GetCamera()->GetMinTargetDistance()) + 1.f;
  float dist = Game::Get()->GetCamera()->GetDistToTarget();
  float ratio = (dist / range) * Game::Get()->GetSession()->mMouseZoomDamp;

  Game::Get()->GetCamera()->OrbitX(Game::Get()->GetInput()->MouseXDelta() / Game::Get()->GetSession()->mMouseSpeed);
  Game::Get()->GetCamera()->OrbitY(Game::Get()->GetInput()->MouseYDelta() / Game::Get()->GetSession()->mMouseSpeed);
  Game::Get()->GetCamera()->TrackZ(Game::Get()->GetInput()->MouseZDelta() * ratio);
}

/*  ________________________________________________________________________ */
bool Generic_KeyDownInt(int key)
/*! Key down interrupt for general game session interaction.

    @param key  The scancode of the key that was hit.
    
    @return
    True if the key was handled, false if not.
*/
{
UIEditText *chatBox = static_cast< UIEditText* >(Game::Get()->GetScreen()->GetElement(kUI_GPChatBoxName));
  
  // If a menu is present, either the menu screen handles the key,
  // or the key was Escape and the menu screen goes away.
  if(Game::Get()->GetSession()->IsMenuOpen())
  {
    if(key == DIK_F1 || key == DIK_ESCAPE)
    {
      Game::Get()->GetSession()->HideMenu();
      return (true);
    }
    else
    {
      // Let the menu take it, but return true regardless
      // so that interaction with the rest of the game is off
      // while the menu is up.
      Game::Get()->GetSession()->GetMenu()->CheckKeyDown(key);
      return (true);
    }
  }

  // First let potential UI elements have a shot at the key.
  // If the chat box is open, it eats all keystrokes (except
  // the one that closes the box).
  if(!chatBox->IsDead() && key != DIK_RETURN)
  {
    chatBox->CheckKeyDown(key);
    return (true);
  }
  
  if(key == DIK_RETURN)
  {
    // Show or hide the chat box.
    if(!chatBox->IsDead())
    {
      // When we close the box, we send whatever's in it.
      if(!chatBox->GetText().empty())
      {
      std::string  chat = chatBox->GetText();
      
        NetClientSendChat(chat);
      
        // Empty it.
        chatBox->SetText("");
      }
      chatBox->Die(true);
      chatBox->Exit();
    }
    else
    {
      chatBox->Die(false);
      chatBox->Reset();
    }
    return (true);
  }
  if(key == DIK_SYSRQ)
  {
    Game::Get()->GetWindow()->GetRenderer()->TakeScreenshot();
    return (true);
  }
  if(key == DIK_S)
  {
    if (Game::Get()->GetSession()->GetRules()->GetGameType() != NINETEEN_BALL)
      UI_GPSBToggleClick();
  }
  if(key == DIK_J)
    UI_GPJBToggleClick();
  if(key == DIK_ESCAPE)
  {
    if(!(Game::Get()->GetPhysics()->AtRest())) 
        scast<UIPanel*>(Game::Get()->GetScreen()->GetElement(kUI_GPMenuPanelName))->GetElement("ResignButton")->Enable(false);
    else
        scast<UIPanel*>(Game::Get()->GetScreen()->GetElement(kUI_GPMenuPanelName))->GetElement("ResignButton")->Enable(Game::Get()->GetSession()->CurrentTurn() == Game::Get()->GetMyTurn());
    
    Game::Get()->GetSession()->ShowMenu(kUI_GPMenuPanelName);
    return (true);
  }
  if(key == DIK_F1)
  {
    // Show the help menu.
    Game::Get()->GetSession()->ShowMenu(kUI_GPHelpPanelName);
    return (true);
  }
  if(key == DIK_M)
    Game::Get()->DrawMessages(!Game::Get()->DrawMessages());
  if(key == DIK_T)
  {
    Game::Get()->GetSession()->mTutorVisible = !Game::Get()->GetSession()->mTutorVisible;
    if(Game::Get()->GetSession()->mTutorVisible)
    {
      Game::Get()->GetScreen()->GetElement(kUI_GPTutorName)->Die(false);
      Game::Get()->GetScreen()->GetElement(kUI_GPTutorName)->Reset();
    }
    else
    {
      Game::Get()->GetScreen()->GetElement(kUI_GPTutorName)->Die(true);
      Game::Get()->GetScreen()->GetElement(kUI_GPTutorName)->Exit();
    }
  }
  if(key == DIK_SPACE)
  {
    Game::Get()->DrawLitBalls(!Game::Get()->DrawLitBalls());
  }
  if(key == DIK_TAB)
  {
    Game::Get()->GetSession()->mCamLocked = !Game::Get()->GetSession()->mCamLocked;
    if(Game::Get()->GetSession()->mCamLocked)
    {
      ::ShowCursor(true);
      Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Die(false);
      Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Reset();
    }
    else
    {
      ::ShowCursor(false);
      Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Die(true);
      Game::Get()->GetScreen()->GetElement(kUI_GPCamLockName)->Exit();
    }
  }
  
  // Didn't handle it.
  return (false);
}

/*  ________________________________________________________________________ */
void ShotLineup_KeyDownInt(int key)
/*! Key down interrupt for shot lineup.

    @param key  The scancode of the key that was hit.
*/
{
Game        *game    = Game::Get();
GameSession *session = game->GetSession();

  // Call the general handler.
  if(Generic_KeyDownInt(key))
    return;

  if(key == DIK_L)
  {
    session->mShotLocked = !session->mShotLocked;
    if(session->mShotLocked)
    {
      game->GetCamera()->GetPosVector(&session->mLockedPos);
      game->GetScreen()->GetElement(kUI_GPCueLockName)->Die(false);
      game->GetScreen()->GetElement(kUI_GPCueLockName)->Reset();
    }
    else
    {
      game->GetCamera()->SetPosition(session->mLockedPos.x,session->mLockedPos.y,session->mLockedPos.z);
      game->GetScreen()->GetElement(kUI_GPCueLockName)->Die(true);
      game->GetScreen()->GetElement(kUI_GPCueLockName)->Exit();
    }
  }
  if(key == DIK_G)
  {
    Game::Get()->DrawGhost( !Game::Get()->DrawGhost() );
  }
  //if(key == DIK_C)
  //{
  //  session->TransitionTo(session->CallShotSID());
  //}
}

/*  ________________________________________________________________________ */
void BallInHand_KeyDownInt(int key)
/*! Key down interrupt for shot lineup.

    @param key  The scancode of the key that was hit.
*/
{
//Game       *game     = Game::Get();
//GameSession *session = game->GetSession();

  // Call the general handler.  
  if(Generic_KeyDownInt(key))
    return;
}

/*  ________________________________________________________________________ */
void PowerResolve_KeyDownInt(int key)
/*! Key down interrupt for shot lineup.

    @param key  The scancode of the key that was hit.
*/
{  
  // Call the general handler.
  if(Generic_KeyDownInt(key))
    return;
}

/*  ________________________________________________________________________ */
void WatchShot_KeyDownInt(int key)
/*! Key down interrupt for shot viewing.

    @param key  The scancode of the key that was hit.
*/
{  
  // Call the general handler.
  if(Generic_KeyDownInt(key))
    return;
}

/*  ________________________________________________________________________ */
void CallShot_KeyDownInt(int key)
/*! Key down interrupt for shot calling.

    @param key  The scancode of the key that was hit.
*/
{  
  // Call the general handler.
  if(Generic_KeyDownInt(key))
    return;
}

/*  ________________________________________________________________________ */
bool Generic_LeftClickInt(int x,int y,int /*z*/)
/*! Generic left click interrupt for game session.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
    
    @return
    True if the click was handled, false if not.
*/
{
  // Let the UI handle it.
  if(Game::Get()->GetScreen()->CheckLeftClick(x,y))
    return (true);
  
  return (false);
}

/*  ________________________________________________________________________ */
void ShotLineup_LeftClickInt(int x,int y,int z)
/*! Left click interrupt for shot lineup.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
Game        *game = Game::Get();
GameSession *session = game->GetSession();

  if(Generic_LeftClickInt(x,y,z))
    return;

  if(game->GetPhysics()->AtRest() && game->IsMyTurn())
    session->TransitionTo(session->PowerResolveSID());
}

/*  ________________________________________________________________________ */
void PowerResolve_LeftClickInt(int x,int y,int z)
/*! Left click interrupt for power resolution.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
Game *game = Game::Get();

  if(Generic_LeftClickInt(x,y,z))
    return;
    
  if(game->GetPhysics()->AtRest() && game->IsMyTurn())
  {
  float  power = game->GetPhysics()->GetConfigValue("Limits","MaxLinearVelocity",Physics::kDef_MaxLinearVel);
    
    // Scale power by whatever the power meter indicates.
    power *= static_cast< UIPowerMeter* >(game->GetScreen()->GetElement("PowerMeter"))->GetPower();
    
    // Do it.
	  NetClientSendTurn(game->GetMyShotVector(),power);
  }

}

/*  ________________________________________________________________________ */
void WatchShot_LeftClickInt(int x,int y,int z)
/*! Left click interrupt for shot resolution / watching.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
  if(!Generic_LeftClickInt(x,y,z))
  {
  }
}

/*  ________________________________________________________________________ */
void CallShot_LeftClickInt(int x,int y,int z)
/*! Left click interrupt for shot calling.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
Game        *game = Game::Get();
GameSession *session = game->GetSession();

  if(Generic_LeftClickInt(x,y,z))
    return;

  session->TransitionTo(session->ShotLineupSID());
}

/*  ________________________________________________________________________ */
void BallInHand_LeftClickInt(int x,int y,int z)
/*! Left click interrupt for ball in hand.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
  if(Generic_LeftClickInt(x,y,z))
    return;
  if(Game::Get()->GetSession()->mCamLocked || Game::Get()->GetSession()->IsMenuOpen())
    return;
  
  // Adjust the cue ball.
Geometry::Vector3D  cueBall = Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(),Physics::Engine::eRigidBodyVector::propPosition);
  
  NetClientSendCueAdjust(cueBall[0],cueBall[1],cueBall[2]);
  Game::Get()->GetSession()->TransitionTo(Game::Get()->GetSession()->ShotLineupSID());
}

/*  ________________________________________________________________________ */
void BallInHand_RightClickInt(int x,int y,int z)
/*! Right click interrupt for ball in hand.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
  if(Game::Get()->GetSession()->mCamLocked || Game::Get()->GetSession()->IsMenuOpen())
    return;

  if(!Generic_RightClickInt(x,y,z))
  {
  }
  
  if(gBIHAnimTime <= 0.0f)
  {
    gBIHAnimTime = 0.0f;
    gBIHAnimStep = 1.0f;
    gBIHAnimate = true;
  }
  else if(gBIHAnimTime >= 1.0f)
  {
    gBIHAnimTime = 1.0f;
    gBIHAnimStep = -1.0f;
    gBIHAnimate = true;
  }
}

/*  ________________________________________________________________________ */
bool Generic_RightClickInt(int /*x*/,int /*y*/,int /*z*/)
/*! Generic right click interrupt for game session.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
    
    @return
    True if the click was handled, false if not.
*/
{
  return (false);
}

/*  ________________________________________________________________________ */
void PowerResolve_RightClickInt(int x,int y,int z)
/*! Right click interrupt for power resolution.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
Game        *game    = Game::Get();
GameSession *session = game->GetSession();

  if(Generic_RightClickInt(x,y,z))
    return;

  if(game->GetPhysics()->AtRest() && game->IsMyTurn())
    session->TransitionTo(session->ShotLineupSID());
}

/*  ________________________________________________________________________ */
void CallShot_RightClickInt(int x,int y,int z)
/*! Right click interrupt for shot calling.

    @param x  X coordinate of the mouse during the click.
    @param y  Y coordinate of the mouse during the click.
    @param z  Z coordinate of the mouse during the click.
*/
{
Game        *game = Game::Get();
GameSession *session = game->GetSession();

  if(Generic_RightClickInt(x,y,z))
    return;

  session->TransitionTo(session->ShotLineupSID());
}