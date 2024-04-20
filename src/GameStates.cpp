/* =========================================================================
   
    @file    GameStates.h
    @author  jmp
    @brief   Game state functions.
    
   ========================================================================= */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "Window.h"

#include "Input.h"
#include "UIScreen.h"
#include "UIElement.h"
#include "UIGraphic.h"
#include "UIButton.h"
#include "UIEditText.h"
#include "UIPanel.h"
#include "UIPowerMeter.h"
#include "UIListbox.h"
#include "UIMenu.h"
#include "Clock.h"

#include "NetTracker.h"
#include "NetGameDiscovery.h"
#include "AIPlayer.h"

#include "Log.h"
#include "RuleSystem.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

int gTransferOnExit = -1;  // State to transfer to when exit animations complete.
Clock splashClock;

/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
void ExitFinished(void)
/*! Callback invoked when a screen finishes exit transitions.
*/
{
  // Transfer to the given state.
  if(gTransferOnExit >= 0)
  {
    Game::Get()->TransitionTo(gTransferOnExit);
    gTransferOnExit = -1;
  }
}

/*  ________________________________________________________________________ */
void GameState_Splash1Enter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for splash screen one.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Make sure we have the correct input handlers.
  game->GetInput()->InstallKeyDownHandler(SplashKeyInt);
  game->GetInput()->InstallLeftClickHandler(SplashClickInt);
  
  // Set the screen.
  game->SetActiveScreen("Splash1");
  game->GetScreen()->Reset();
  
  splashClock.Reset();
}

/*  ________________________________________________________________________ */
void GameState_Splash1Exit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for splash screen one.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
}

/*  ________________________________________________________________________ */
void GameState_Splash1Update(StateMachine *sm,float /*elapsed*/)
/*! State update function for splash screen one..

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  splashClock.Update();
  if(splashClock.Accumulated() >= 2.0f)
    sm->TransitionTo(Game::Get()->splash2SID);
}

/*  ________________________________________________________________________ */
void GameState_Splash2Enter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for splash screen one.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Make sure we have the correct input handlers.
  game->GetInput()->InstallKeyDownHandler(SplashKeyInt);
  game->GetInput()->InstallLeftClickHandler(SplashClickInt);
  
  // Set the screen.
  game->SetActiveScreen("Splash2");
  game->GetScreen()->Reset();
  
  splashClock.Reset();
}

/*  ________________________________________________________________________ */
void GameState_Splash2Exit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for splash screen one.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
}

/*  ________________________________________________________________________ */
void GameState_Splash2Update(StateMachine *sm,float /*elapsed*/)
/*! State update function for splash screen one..

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  splashClock.Update();
  if(splashClock.Accumulated() >= 2.0f)
    sm->TransitionTo(Game::Get()->GetMainMenuStateID());
}

/*  ________________________________________________________________________ */
void GameState_MainMenuEnter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for main menu.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Make sure we have the correct input handlers.
  game->GetInput()->InstallKeyDownHandler(KeyHandler_MainMenu);
  game->GetInput()->InstallLeftClickHandler(LCHandler_MainMenu);
  
  // Set the screen.
  game->SetActiveScreen("MainMenu");
  game->GetScreen()->Reset();

  // Position the camera.
  game->GetCamera()->SetTarget(0.0f,0.0f,0.0f,false);
  game->GetCamera()->SetPosition(0.0f,0.0f,-10.0f,false);
  game->GetCamera()->TrackZ(-100.0f);
  game->GetCamera()->OrbitY(25.0f);	

  //int i;
  
  //i = game->GetSound()->Load2DObject("data/sound/welcome.wav");
  
  //game->GetSound()->PlayObject(i);
}

/*  ________________________________________________________________________ */
void GameState_MainMenuExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for main menu.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
}

/*  ________________________________________________________________________ */
void GameState_MainMenuUpdate(StateMachine *sm,float elapsed)
/*! State update function for main menu.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // In the menu, we move the camera in a circle.
  game->GetCamera()->OrbitX(kCam_Speed * elapsed);
  

  // Update the arena.
  game->GetPlayfield()->Update(game->GetPhysics());
}

/*  ________________________________________________________________________ */
void GameState_GameSelectEnter(StateMachine * sm,float /*elapsed*/)
/*! State setup function for game select screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game      *game = static_cast< Game* >(sm);

  // Set up game discovery.
  InitDiscoveryInterface();
  
  // Set up the screen.
  game->SetActiveScreen("GameSelect");
  game->GetScreen()->Reset();
}

/*  ________________________________________________________________________ */
void GameState_GameSelectExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for game select screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  // Stop game discovery.
  KillDiscoveryInterface();
}

/*  ________________________________________________________________________ */
void GameState_GameSelectUpdate(StateMachine *sm,float elapsed)
/*! State update function for game select screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  Game *game = static_cast< Game* >(sm);

  // Update game discovery information.
  UpdateDiscoveryInterface();

  UIPanel   *p  = reinterpret_cast< UIPanel* >(game->GetScreen()->GetElement(kUI_GSPanelName));
  UIListbox *lb = reinterpret_cast< UIListbox* >(p->GetElement(kUI_GSListName));
  NetGameItr g  = NetGamesBegin();

  lb->ClearItems();
  
  if(lb->GetSelected() >= scast< signed int >(NetGamesSize()))
      lb->SetSelected(-1);

  if(g == NetGamesEnd())
      lb->SetSelected(-1);
  else
  {

    while(g != NetGamesEnd())
    {
        // Games are not fully resolved until we have their name and IP address.
        if(!g->second.name.empty() && !g->second.address.empty())
        lb->AddItem(g->second.name);
        ++g;
    }
  }

    // Update panel and join button based on whether a game to join is selected or not
  int index = lb->GetSelected() ;
  if ( index >= 0 )
  {
      // Find the appropriate NetGameInfo
    NetGameItr gameInfo = NetGamesBegin() ;
    for ( int i = 0 ; i < index && gameInfo != NetGamesEnd(); ++i )
      gameInfo++ ;

    if ( gameInfo->second.numAvailPlayers > 0 )
      game->GetScreen()->GetElement( kUI_GSJoinButton )->Enable( true ) ;
    else
      game->GetScreen()->GetElement( kUI_GSJoinButton )->Enable( false ) ;

      // Set panel values
    UIEditText * e ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameName ) ) ;
    if(NetGamesBegin() == NetGamesEnd())
    {
        __asm nop
    }
    e->SetText( gameInfo->second.name ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameType ) ) ;
	e->SetText( GameTypes[ gameInfo->second.gameType ] ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameHost ) ) ;
    e->SetText( gameInfo->second.hostPlayerName ) ;

      // Human and AI players are more complicated
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameHumanP ) ) ;
    std::ostringstream humanString ;
    humanString << gameInfo->second.numHumanPlayers ;
    e->SetText( humanString.str() ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameAIP ) ) ;
	std::ostringstream aiString ;
    aiString << gameInfo->second.numAIPlayers ;
    e->SetText( aiString.str() ) ;

      // Available slots is much more complicated
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameAvailP ) ) ;
    if ( gameInfo->second.numAvailPlayers > 0 )
    {
      std::ostringstream string ;
      string << kUI_GSGameAvailCapS << gameInfo->second.numAvailPlayers << kUI_GSGameAvailCapE ;
      e->SetText( string.str() ) ;
    }
    else
    {
      e->SetText( kUI_GSGameAvailFullCap ) ;
    }
  }
  else
  {
    game->GetScreen()->GetElement( kUI_GSJoinButton )->Enable( false ) ;

      // Reset panel
    UIEditText * e ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameName ) ) ;
    e->SetText( kUI_GSGameNameText ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameType ) ) ;
    e->SetText( "" ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameHost ) ) ;
    e->SetText( "" ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameHumanP ) ) ;
    e->SetText( "" ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameAIP ) ) ;
    e->SetText( "" ) ;
    e = scast< UIEditText * >( p->GetElement( kUI_GSSelGameAvailP ) ) ;
    e->SetText( "" ) ;
  }

  //lb->ClearItems();
  //while(g != NetGamesEnd())
  //{
  //  // Games are not fully resolved until we have their name and IP address.
  //  if(!g->second.name.empty() && !g->second.address.empty())
  //    lb->AddItem(g->second.name);
  //  ++g;
  //}

  // In the menu, we move the camera in a circle.
  game->GetCamera()->OrbitX(kCam_Speed * elapsed);

  // Update the arena.
  game->GetPlayfield()->Update(game->GetPhysics());
}

/*  ________________________________________________________________________ */
void GameState_GameOptionsEnter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for game options screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game        *game = static_cast< Game* >(sm);
UIElement   *e    = 0;
UIPanel     *p    = 0;
  
  // Reset screen to initial values
  game->RemoveScreen("GameOptions");
  game->nInitGOUI();

  // Set up the screen.
  game->SetActiveScreen("GameOptions");
  game->GetScreen()->Reset();
  
  // Create the game session.
  // If we're hosting the game, this will set up server stuff appropriately.
  // Otherwise, we're joining the game, and this will establish our connection.
std::string  serverName = game->GetMyServer();
short        serverPort = game->GetMyPort();

  if(serverName.empty())
    game->CreateSession(true);
  else
    game->CreateSession(false,serverName,serverPort);

    // Initial game name
  UIEditText * gameName = scast< UIEditText * >( scast< UIPanel * >( game->GetScreen()->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOTitleName ) ) ;
  game->GetSession()->SetGameName( gameName->GetText() ) ;

    // Set up UI for a newly-created game
  if (game->GetSession()->IsHost())
  {
    UI_GOGameType1Click() ;
    UI_GOPButtonAddAIClick( 1 ) ;
  }

  // If we're not the host, we need to disable most of the controls on this screen.
  bool  enabled = serverName.empty();
  p = reinterpret_cast< UIPanel* >(game->GetScreen()->GetElement(kUI_GOPanelName));
  e = p->GetElement(kUI_GOTitleName);
  e->Enable(enabled);
  for ( unsigned int i = 1 ; i < GameMaxPlayers[ 0 ] ; ++i )
  {
	e = p->GetElement(kUI_GOPKickButtonName[i]);
	e->Enable(enabled);
	e = p->GetElement(kUI_GOPAddAIButtonName[i]);
	if ( i == 1 )
		e->Enable( false ) ;
	else
		e->Enable(enabled);
  }
  for ( unsigned int i = GameMaxPlayers[ 0 ] ; i < kPlayersMax ; ++i )
  {
    e = p->GetElement(kUI_GOPButtonName[i]);
	e->Enable(false);
	e = p->GetElement(kUI_GOPKickButtonName[i]);
	e->Enable(false);
	e = p->GetElement(kUI_GOPAddAIButtonName[i]);
	e->Enable(false);
  }
  for ( int i = 0 ; i < GAME_TYPE_COUNT ; ++i )
  {
	e = p->GetElement(kUI_GOGameTypeBtnName[i]);
	e->Enable(enabled);
  }
  e = game->GetScreen()->GetElement(kUI_GOStartBtnCap);
  e->Enable(enabled);
}

/*  ________________________________________________________________________ */
void GameState_GameOptionsExit(StateMachine * sm,float /*elapsed*/)
/*! State cleanup function for game select screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game        *game = static_cast< Game* >(sm);
UIElement   *e    = 0;

  e = game->GetScreen("GameOptions")->GetElement(kUI_GOStartBtnCap);
  e->Enable(true);
}

/*  ________________________________________________________________________ */
void GameState_GameOptionsUpdate(StateMachine *sm,float elapsed)
/*! State update function for game select screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
  Game    *game = static_cast< Game* >(sm);
  UIPanel *p    = reinterpret_cast< UIPanel* >(game->GetScreen()->GetElement(kUI_GOPanelName));

    // Update Broadcast
  if ( game != 0 && game->GetSession() !=0 && game->GetSession()->IsHost() )
  {
		// Find player counts (for update broadcast)
	int numHumanPlayers = 0 ;
	int numAIPlayers = 0 ;
	int numAvailPlayers = GameMaxPlayers[ gCurrentGameType ] ;
	for ( int i = 0 ; i < scast< int >( GameMaxPlayers[ gCurrentGameType ] ) ; ++i )
	{
		if ( game->GetSession()->GetPlayer( i ) != 0 )
		{
				// Human or AI
			if ( game->GetSession()->GetPlayer( i )->IsAI() )
				numAIPlayers++ ;
			else
				numHumanPlayers++ ;

			numAvailPlayers-- ;
		}
		else
		{
				// Closed or available
			UIButton * et = scast< UIButton * >( p->GetElement( kUI_GOPButtonName[ i ] ) ) ;
			if ( et->GetCaption() == kUI_GOSlotCloseBtnCap )
				numAvailPlayers-- ;
		}
	}

	// Update game information broadcast.
	NetGameInfo netInfo ;
	netInfo.name = scast< UIEditText * >( p->GetElement( kUI_GOTitleName ) )->GetText() ;
	netInfo.gameType = scast< eGameType >( gCurrentGameType ) ;
    netInfo.hostPlayerName = game->GetMyName() ;
	netInfo.numHumanPlayers = numHumanPlayers ;
	netInfo.numAIPlayers = numAIPlayers ;
	netInfo.numAvailPlayers = numAvailPlayers ;
	NetGameRegUpdate( netInfo );
  }

  if ( game && game->GetSession() && !game->GetSession()->IsHost() )
  {
      // Modify UI based on current state
    UIElement * e ;
    e = p->GetElement( kUI_GOTitleName ) ;
    scast< UIEditText * >( e )->SetText( Game::Get()->GetSession()->GetGameName() ) ;
    for ( int i = 0 ; i < kPlayersMax ; ++i )
    {
        e = p->GetElement( kUI_GOPButtonName[ i ] ) ;
	    if ( Game::Get()->GetSession()->GetPlayer( i ) != 0 )
          scast< UIButton * >( e )->SetCaption( Game::Get()->GetSession()->GetPlayer( i )->GetName() ) ;
	    else
	      scast< UIButton * >( e )->SetCaption( "(empty)" ) ;
    }
	e = p->GetElement( kUI_GOGameTypeBtnName[ gCurrentGameType ] ) ;
	UIPanel * ePanel = scast< UIPanel * >( game->GetScreen()->GetElement( kUI_GOGameTypePnlName[ gCurrentGameType ] ) ) ;
	if ( ePanel->IsDead() )
		UI_GOGameTypeClick[ gCurrentGameType ]();
  }
  else if ( game && game->GetSession() && game->GetSession()->IsHost() )
  {
	static unsigned int delay = 100 ;

      // Send updated game options
    if ( --delay == 0 )
    {
	  delay = 100 ;

      PacketGameOptions  po;
      po.gameName  = scast< UIEditText * >( p->GetElement( kUI_GOTitleName ) )->GetText() ;
      po.playerCur = Game::Get()->GetSession()->GetPlayersCur() + 1 ;
      po.playerMax = Game::Get()->GetSession()->GetPlayersMax() ;
      po.gameType  = gCurrentGameType ;

      for( int i = 0 ; i < kPlayersMax ; ++i )
      {
        PacketGameOptions::PlayerInfo  info ;
        Player *                       p = Game::Get()->GetSession()->GetPlayer( i ) ;

        if( p == 0 )
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
        else if ( p->IsAI() )
        {
          info.name = kUI_GOSlotAIBtnCap ;
          info.type = kPlayerType_AI ;
        }
        else
        {
          info.name = p->GetName() ;
          info.type = kPlayerType_Human ;
        }

        po.players.push_back( info ) ;
      }

      NetServerSendGameOptions( po ) ;
    }
  }

  // Erase the player names list box and refill it with the current list.
  //UIListbox *lb = reinterpret_cast< UIListbox* >(p->GetElement(kUI_GOPlayersName));

  //lb->ClearItems();
  //for(int i = 0; i < game->GetSession()->GetPlayersCur(); ++i)
  //  lb->AddItem(game->GetSession()->GetPlayer(i));

  // In the menu, we move the camera in a circle.
  game->GetCamera()->OrbitX(kCam_Speed * elapsed);

  // Update the arena.
  game->GetPlayfield()->Update(game->GetPhysics());
}

/*  ________________________________________________________________________ */
void GameState_GameplayEnter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for gameplay.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Reset screen to initial values
  game->RemoveScreen("Gameplay");
  game->nInitGPUI();
  UIPanel *   jukebox  = rcast< UIPanel * >( Game::Get()->GetScreen( "Gameplay" )->GetElement( kUI_GPJukeboxPanelName ) ) ;
  UIListbox * playlist = rcast< UIListbox * >( jukebox->GetElement( kUI_GPJBPlaylistName ) ) ;
  for ( unsigned int i = 0 ; i < game->GetJBSongs().size() ; ++i )
    playlist->AddItem( (game->GetJBSongs())[ i ].m_songFilename ) ;
  
  // Set the screen.
  game->SetActiveScreen("Gameplay");
  game->GetScreen()->Reset();

  // Go to appropriate screen.
  if(game->GetSession()->IsHost())
    game->GetSession()->TransitionTo(game->GetSession()->BallInHandSID());
  else
  {
  Geometry::Vector3D  cbv = Game::Get()->GetPlayfield()->GetBallPosition(Game::Get()->GetPhysics(),0);

  /////@todo this is hacky, should be elsewhere or encapsulated
  //D3DXVECTOR3  camPos;
  //D3DXVECTOR3  camTarget;
  //D3DXVECTOR3  shotVec(vx,vy,vz);
  //D3DXVECTOR3  cueBall(cbv[0],cbv[1],cbv[2]);

  //  // We'll need these.
  //  Game::Get()->GetCamera()->GetPosVector(&camPos);
  //  Game::Get()->GetCamera()->GetTargetVector(&camTarget);
  //  
  //  // The shot vector should already be normalized for us.
  //  Game::Get()->GetShotAnimData().vec        = shotVec;
  //  Game::Get()->GetShotAnimData().power      = power;
  //  Game::Get()->GetShotAnimData().taken      = false;
  //  Game::Get()->GetShotAnimData().cueTip     = cueBall + (-shotVec * 10.0f);
  //  Game::Get()->GetShotAnimData().cueButt    = cueBall + (-shotVec * 60.0f);
  //  Game::Get()->GetShotAnimData().cueTime    = 0.0f;
  //  Game::Get()->GetShotAnimData().cueStrokes = 0;
  //  
  //  Game::Get()->GetGlideAnimData().eyeStart    = camPos;
  //  Game::Get()->GetGlideAnimData().eyeStop     = D3DXVECTOR3(-100.0f,80.0f,-10.0f);
  //  Game::Get()->GetGlideAnimData().targetStart = camTarget;
  //  Game::Get()->GetGlideAnimData().targetStop  = D3DXVECTOR3(0.0f,0.0f,0.0f);
  //  Game::Get()->GetGlideAnimData().time        = 0.0f;	  
  //  
    game->GetSession()->TransitionTo(game->GetSession()->ShotLineupSID());
  }
     
  // Position the camera.
  Geometry::Vector3D pos = game->GetPlayfield()->GetBallPosition(game->GetPhysics(), 0);
  
  game->GetCamera()->SetTarget(pos[0],pos[1],pos[2]);
  game->GetCamera()->SetPosition(0.0f,0.0f,-50.0f);
  
  // Print the welcome message.
  game->WriteMessage("Welcome to Corner Hooked! Press F1 for help.");
  
  // Bring out the standard UI elements.
  game->GetScreen()->GetElement(kUI_GPStatusPanelName)->Die(false);
  game->GetScreen()->GetElement(kUI_GPStatusPanelName)->Reset();
  
  // Indicators.
  Game::Get()->GetScreen()->GetElement(kUI_GPCamIndName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPCamIndName)->Reset();
  Game::Get()->GetScreen()->GetElement(kUI_GPCueIndName)->Die(false);
  Game::Get()->GetScreen()->GetElement(kUI_GPCueIndName)->Reset();

  // No scoring for 19-ball
  UIElement * sb = Game::Get()->GetScreen()->GetElement(kUI_GPScoreboardToggleName);
  if (Game::Get()->GetSession()->GetRules()->GetGameType() == NINETEEN_BALL)
    sb->Enable(false);
  else
    sb->Enable(true);

  // Finally, hide the cursor.
  ::ShowCursor(false);
}

/*  ________________________________________________________________________ */
void GameState_GameplayExit(StateMachine *sm,float /*elapsed*/)
/*! State cleanup function for gameplay.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Show the cursor.
  ::ShowCursor(true);

  // Clean up game session.
  game->DestroySession();
}

/*  ________________________________________________________________________ */
void GameState_GameplayUpdate(StateMachine *sm,float elapsed)
/*! State update function for gameplay.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game              *game = static_cast< Game* >(sm);
UIPanel           *p    = 0;
UIEditText        *e    = 0;
std::stringstream  sn_fmt;

  // Force the stupid mouse to go hide.
CURSORINFO  ci;

  GetCursorInfo(&ci);
  if(Game::Get()->GetSession()->mCamLocked || Game::Get()->GetSession()->IsMenuOpen())
  {
    while(ci.flags == 0)
      ::ShowCursor(true);
  }
  else
  {
    while(ci.flags & CURSOR_SHOWING)
      ::ShowCursor(false);
  } 



std::string ss = "";
if(Game::Get()->GetSession()->GetRules()->GetGameType() == EIGHTEEN_BALL)
{
 if(static_cast< Rules_EighteenBall * >(Game::Get()->GetSession()->GetRules())->GetGroup() == OPEN)
     ss = "OPEN";
 if(static_cast< Rules_EighteenBall * >(Game::Get()->GetSession()->GetRules())->GetGroup() == SOLIDS)
    ss = "SOLIDS";
 if(static_cast< Rules_EighteenBall * >(Game::Get()->GetSession()->GetRules())->GetGroup() == STRIPES)
     ss = "STRIPES";
 if(static_cast< Rules_EighteenBall * >(Game::Get()->GetSession()->GetRules())->GetGroup() == EIGHTEEN)
     ss = "EIGHTEEN";
}
//else if(Game::Get()->GetSession()->GetRules()->GetGameType() == NINETEEN_BALL)
//{
//    std::stringstream balls;
//
//    balls << Game::Get()->GetSession()->GetRules()->GetLegalBalls(0)[0];
//    ss = balls.str();
//}
  // Update the status panel.
  p = static_cast< UIPanel* >(game->GetScreen()->GetElement(kUI_GPStatusPanelName));
  e = static_cast< UIEditText* >(p->GetElement(kUI_GPStatusTextName));
  
std::vector< int >  legalBalls = Game::Get()->GetSession()->GetRules()->GetLegalBalls(Game::Get()->GetSession()->CurrentTurn());
bool                lookAtLegal = false;


  if(std::find(legalBalls.begin(),legalBalls.end(),Game::Get()->lookingAtBall) != legalBalls.end() &&
     Game::Get()->lookingAtBall != 0)
  {
    lookAtLegal = true;
  }
  
  sn_fmt << "Shooting Now: " << Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn())->GetName();
  sn_fmt << "    " << ss;
  if(Game::Get()->GetMyTurn() == Game::Get()->GetSession()->CurrentTurn())
  {
    if(Game::Get()->lookingAtBall == 0)
    {
        //sn_fmt << "Shooting Now: " << Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn())->GetName() << " " << ss
        sn_fmt << "\nLooking at: nothing";
    }
    else
    {
        //sn_fmt << "Shooting Now: " << Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn())->GetName() << " " << ss
        sn_fmt << "\nLooking at: " << Game::Get()->lookingAtBall << "-ball (" << (lookAtLegal ? "a" : "not a") << " legal ball)";
    }   
  }
  e->SetText(sn_fmt.str());
  
	if(game->GetPhysics()->AtRest())
	{
		if(game->GetPlayfield()->mPocketedBalls.size())
		{
			for(size_t i = 0; i < game->GetPlayfield()->mPocketedBalls.size(); ++i)
			{
				if(game->GetPlayfield()->mPocketedBalls[i] == game->GetPlayfield()->mBalls[0]->ID())
				{
					//game->GetPlayfield()->mBalls[0]->ID(game->GetPhysics()->AddRigidBodySphere(1));
					game->GetPhysics()->RigidBodyVector3D(game->GetPlayfield()->mBalls[0]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, -25));
					game->GetPhysics()->RigidBodyBool(game->GetPlayfield()->mBalls[0]->ID(), Physics::Engine::eRigidBodyBool::propCollidable, true);
					game->GetPhysics()->RigidBodyBool(game->GetPlayfield()->mBalls[0]->ID(), Physics::Engine::eRigidBodyBool::propActive, true);
					game->GetPhysics()->RigidBodyBool(game->GetPlayfield()->mBalls[0]->ID(), Physics::Engine::eRigidBodyBool::propTranslatable, true);
					game->GetPhysics()->RigidBodyBool(game->GetPlayfield()->mBalls[0]->ID(), Physics::Engine::eRigidBodyBool::propUseGravity, false);
					game->GetPhysics()->RigidBodyScalar(game->GetPlayfield()->mBalls[0]->ID(), Physics::Engine::eRigidBodyScalar::propMass, 1.0);
				}
			}
			game->GetPlayfield()->mPocketedBalls.clear();	
		}
		// so we don't make it every frame
		static Geometry::Vector3D look_at; 

		/*Shot shot = game->GetAIPlayer()->SelectShot();
		game->GetPlayfield()->mGhostBall->SphereMesh()->SetTranslation(shot.p[0], shot.p[1], shot.p[2]);*/

		// look at the cue ball
		look_at = game->GetPlayfield()->GetBallPosition(game->GetPhysics(), 0);
	//	game->GetCamera()->SetTarget(look_at[0], look_at[1], look_at[2]);
	}

    
  // Update the physics.
  float dt = .05f;//elapsed;//clock.Elapsed();//.2;
	int steps = 5;

    std::stringstream msg;
      
    static Clock clock;
    float time = 0.f;
    clock.Reset();
      clock.Update();
  static int frame = 0;

  // don't update the game every frame.
  if(frame++ % 10)
  {
    if(!game->GetPhysics()->AtRest())
    {
        //clock.Reset();
        //clock.Update();
          
        game->GetPhysics()->Update(dt,steps);
        
        //clock.Update();
        //time = clock.Accumulated();
    }
  }
  
    if(game->GetPhysics()->AtRest())
    {
        msg << "At Rest";
    }
    else
    {
        msg << "Moving";
    }
    clock.Update();
    time = clock.Accumulated();
 //msg << time;
 //game->WriteMessage(msg.str());

    for(size_t i = 0; i < game->GetPlayfield()->mPocketedBalls.size(); ++i)
    {
        int aiBall = 0;
        for(unsigned int j = 0; j < game->GetPlayfield()->mBalls.size(); ++j)
        {
            if(game->GetPlayfield()->mBalls[j] && !game->GetPlayfield()->mBalls[j]->Pocketed() && game->GetPlayfield()->mBalls[j]->ID() == game->GetPlayfield()->mPocketedBalls[i])
	        {
                
                   
		        /*game->GetAIPhysics()->RemoveRigidBody(*(game->GetPlayfield()->mAIBalls.begin() + j));
                (*(game->GetPlayfield()->mBalls.begin() + j))->Pocketed(true);*/
                
                if(j == 0)
			        continue;
		        
                /*game->GetPlayfield()->mAIBalls.erase(game->GetPlayfield()->mAIBalls.begin() + j);*/
		        aiBall = j;
	        }
        }
        if(game->GetPlayfield()->mPocketedBalls[i] != GetBallByNumber(0)->ID())
            game->GetPhysics()->RemoveRigidBody(game->GetPlayfield()->mPocketedBalls[i]);
            	  
    }
  
  // If the game is over...
  if(game->GetSession()->GetRules()->GameOver())
  {
	game->UpdateResultPanel() ;
    //game->TransitionTo(game->GetMainMenuStateID());
  }
  
  // Center the cursor.
  if(!game->GetSession()->mCamLocked && 0 == game->GetSession()->GetMenu())
  ::SetCursorPos(game->GetWindow()->Width() / 2,game->GetWindow()->Height() / 2);
  
  // Update the arena.
  game->GetPlayfield()->Update(game->GetPhysics());
  
  // Update the game session.
  game->GetSession()->Update(elapsed);
}

/*  ________________________________________________________________________ */
void GameState_OptionsEnter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for options screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Make sure we have the correct key handler.
  game->GetInput()->InstallKeyDownHandler(KeyHandler_MainMenu);

  // Set the screen.
  game->SetActiveScreen("OptionsVideo");
  game->GetScreen()->Reset();
}

/*  ________________________________________________________________________ */
void GameState_OptionsExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for options screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
}

/*  ________________________________________________________________________ */
void GameState_OptionsUpdate(StateMachine *sm,float elapsed)
/*! State update function for options screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Input processing.  
  if(game->GetInput()->MouseButton(kMB_Left))
    game->GetScreen()->CheckLeftClick(game->GetInput()->MouseX(),game->GetInput()->MouseY());

  // In the menu, we move the camera in a circle.
  game->GetCamera()->OrbitX(kCam_Speed * elapsed);

  // Update the arena.
  game->GetPlayfield()->Update(game->GetPhysics());
}

/*  ________________________________________________________________________ */
void GameState_CreditsEnter(StateMachine *sm,float /*elapsed*/)
/*! State setup function for credits screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Make sure we have the correct key handler.
  game->GetInput()->InstallKeyDownHandler(KeyHandler_MainMenu);

  // Set the screen.
  game->SetActiveScreen("Credits");
  game->GetScreen()->Reset();

  // Display panel
  scast< UIPanel * >( game->GetScreen()->GetElement( kUI_CreditsPanelName ) )->Die( false ) ;
  scast< UIPanel * >( game->GetScreen()->GetElement( kUI_CreditsPanelName ) )->Reset() ;

  // Position the camera.
  game->GetCamera()->SetTarget(0.0f,0.0f,0.0f,false);
  game->GetCamera()->SetPosition(0.0f,0.0f,-10.0f,false);
  game->GetCamera()->TrackZ(-100.0f);
  game->GetCamera()->OrbitY(25.0f);	
}

/*  ________________________________________________________________________ */
void GameState_CreditsExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! State cleanup function for credits screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
}

/*  ________________________________________________________________________ */
void GameState_CreditsUpdate(StateMachine *sm,float elapsed)
/*! State update function for credits screen.

    @param sm       The state machine (convertable to Game).
    @param elapsed  The elapsed time since last call (only for updates).
*/
{
Game *game = static_cast< Game* >(sm);

  // Input processing.  
  if(game->GetInput()->MouseButton(kMB_Left))
    game->GetScreen()->CheckLeftClick(game->GetInput()->MouseX(),game->GetInput()->MouseY());

  // In the menu, we move the camera in a circle.
  game->GetCamera()->OrbitX(kCam_Speed * elapsed);

  // Update the arena.
  game->GetPlayfield()->Update(game->GetPhysics());
}
