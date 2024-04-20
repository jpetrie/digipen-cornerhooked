/*! ========================================================================

     @file    Game.h
     @author  jmc
     @brief   Implementation of core game object.
     
     (c) 2004 DigiPen (USA) Corporation, all rights reserved.

    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"

#include "Player.h"
#include "AIPlayer.h"

#include "DXFont.h"

#include "ball.h"
#include "Input.h"
#include "UIScreen.h"
#include "Particle.h"



/*                                                              globals (!!)
---------------------------------------------------------------------------- */

int gCurrentGameType = 0 ;


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Game::Game(void)
/*! Default constructor.
*/
: ti( 0 ),
  mWindow(new Window),
  mCamera(0), mInput(0),
  mScreen(0),
  mPlayfield(0),
  mPhysics(0),mAIPhysics(0),
  mParticleSystem( new ParticleSystem(256,4) ),
  mSession(0),
  mSkybox(0),
  mFPSFont(0),mMsgFont(0),
  mMsgStart(0),
  mShotOffset(D3DX_PI / 25.0f,D3DX_PI / 25.0f),
  mParticles(0),
  mShotVector(0.0f,0.0f,1.0f),
  mDrawFPS(false),mDrawCue(false),mDrawGhost(false), mDrawMsg(true),mDrawLitBalls(false),
  m_curPlayingIndex(-1),m_curPlayingID(-1),lookingAtBall(0)
{
needToSpot = false;
}

/*  ________________________________________________________________________ */
Game::~Game(void)
/*! Destructor.
*/
{
	// Networking shutdown.
	::WSACleanup();
	
	// Clean up playfield.
	SAFE_DELETE(mPlayfield);

  // Clean up AI player.
  SAFE_DELETE(mAIPlayer);
  
  // Clean up physics.
  SAFE_DELETE(mAIPhysics);
  SAFE_DELETE(mPhysics);
	
	// Delete text renderers.
	SAFE_DELETE(mMsgFont);
	SAFE_DELETE(mFPSFont);
	
	// Delete skybox.
	SAFE_DELETE(mSkybox);
	
	// Delete the camera and input manager.
	SAFE_DELETE(mInput);
	SAFE_DELETE(mCamera);
	
	// Finally, kill the window.
  SAFE_DELETE(mWindow);
}


/*  ________________________________________________________________________ */
void Game::SetActiveScreen(const std::string &screen)
/*! Set the active screen pointer.

    Creates the screen if it does not already exist.
*/
{
std::map< std::string,UIScreen* >::iterator  it = mScreens.find(screen);

  if(it != mScreens.end())
    mScreen = it->second;
  else
  {
  UIScreen *s = new UIScreen();
  
    mScreens[screen] = s;
    mScreen          = s; 
  }
}

/*  ________________________________________________________________________ */
void Game::RemoveScreen(const std::string &screen)
/*! Removes the specified screen.
*/
{
  std::map< std::string,UIScreen* >::iterator  it = mScreens.find(screen);
  if(it != mScreens.end())
    mScreens.erase(it);
}

/*  ________________________________________________________________________ */
void Game::CreateSession(bool host,const std::string &server,short port) 
/*! Construct the game session.

    If host is true, the session is created as a game host (server). Otherwise
    the session is created as a client. The additional parameters contain
    connection information for a client session - they can be ignored and
    allowed to default for a server session.

    @param host    If true, session is created as a game host.
    @param server  If host is false, the IP of the server to connect to.
    @param port    If host is false, the port to connect to the server on.
*/
{
  if ( mSession != 0 )
	  delete mSession ;
  mSession = new GameSession(host,server,port);
}

/*  ________________________________________________________________________ */
void Game::DestroySession(void) 
/*! Destroy the game session.
*/
{
  //ASSERT(0 == mSession); //! @todo why destory it if it is already null?
  SAFE_DELETE(mSession);
}

/*  ________________________________________________________________________ */
void Game::WriteMessage(const std::string &msg)
/*! Write a message to the history buffer.
    
    @param msg  The message to write.
*/
{
  if(mMsgHistory.size() - mMsgStart > kMsgAreaLimit)
    ++mMsgStart;

  // Add the message.
  mMsgHistory.push_back(msg);
  
  // Calculate the lifetime of the oldest message.
  mMsgLifetime = nAdjustMsgLifetime(mMsgHistory[mMsgStart]);
  mMsgTimer.Reset();
}

/*  ________________________________________________________________________ */
void Game::UpdateMessages(void)
/*! Updates the message area.
    One message dissappears every few seconds, depending on the length
    of the message.
*/
{
  mMsgTimer.Update();
  
  // Don't bother if no messages displayed.
  // We don't need to worry about a huge accumulated time
  // here because when we next post a message, the
  // timer will be reset.
  if(mMsgHistory.size() == mMsgStart)
    return;
  
  if(mMsgTimer.Accumulated() >= mMsgLifetime)
  {
    // Increment the start ID to pop the geriatric message
    // off the list.
    ++mMsgStart;
    
    // Get the lifetime of the topmost message.
    mMsgLifetime = nAdjustMsgLifetime(mMsgHistory[mMsgStart]);
    mMsgTimer.Reset();
  }
}

/*  ________________________________________________________________________ */
void Game::PrintMessages(void)
/*! Print the most recent messages to the message area.
*/
{
std::stringstream  fmt;

  for(unsigned int i = mMsgStart; i < mMsgHistory.size(); ++i)
    fmt << mMsgHistory[i] << "\n\n";
  if(Game::Get()->DrawMessages())
  {
    mMsgFont->DrawText(kUI_MsgAreaX + kUIElem_ShadowOffX,kUI_MsgAreaY + kUIElem_ShadowOffY,
                     kUI_MsgAreaW + kUIElem_ShadowOffX,kUI_MsgAreaH + kUIElem_ShadowOffY,
                     0xFF000000,DT_LEFT | DT_WORDBREAK,fmt.str().c_str());
    mMsgFont->DrawText(kUI_MsgAreaX,kUI_MsgAreaY,
                     kUI_MsgAreaW,kUI_MsgAreaH,
                     0xFFFFFFFF,DT_LEFT | DT_WORDBREAK,fmt.str().c_str());
  }
}

/*  ________________________________________________________________________ */
void Game::ClearMessages(void)
/*! Clear the most recent messages from the message area.
    Calling this function will clean up the message display area but will
    not delete any messages.
*/
{
  mMsgStart = mMsgHistory.size();
}

/*  ________________________________________________________________________ */
float Game::nAdjustMsgLifetime(const std::string &msg)
/*! Adjust the lifetime of a message.
    
    @param msg  The message to calculate lifetime for.
    
    @return
    The lifetime of the message.
*/
{
float  life = kMsgAreaLifetime;
  
  // If over the threshold, tweak it a bit.
  if(msg.size() > kMsgAreaAdjThreshold)
  {
  int  adjusts = 1 + (msg.size() - kMsgAreaAdjThreshold) / kMsgAreaCharPerAdj;
  
    // Gives us an additional fraction of a second per every few characters.
    life += static_cast< float >(adjusts) * kMsgAreaTimePerAdj;
  }
  
  // What a beautiful choice.
  return (life);
}

/*  ________________________________________________________________________ */
/*! \brief  Updates the turn indicator if necessary */
void Game::UpdateTurnIndicator()
{
	static const int kLifetime = 180 ;
	static int       lifeRemaining = kLifetime ;

	  // Ignore unless we are in the gameplay screen
	if ( CurrentState() != static_cast<unsigned int>(GetGameplayStateID()) )
		return ;

//	UIPanel * p = rcast< UIPanel * >( GetScreen()->GetElement( kUI_GPTurnIndicatorName ) ) ;
	UIPanel * p = rcast< UIPanel * >( mScreens.find( "Gameplay" )->second->GetElement( kUI_GPTurnIndicatorName ) ) ;

	  // Ignore unless the turn indicator is all the way on the screen
	if ( p->IsInTransition() || p->IsDead() )
	{
		lifeRemaining = kLifetime ;
		return ;
	}

	  // Start its exit transition when enough time has passed
	lifeRemaining-- ;
	if ( lifeRemaining <= 0 )
	{
		p->Exit() ;
		p->Die( true ) ;
	}
}

/*  ________________________________________________________________________ */
/*! \brief  Updates the game end results panel, creating it if necessary */
void Game::UpdateResultPanel( void )
{
	static const int  kLifetime     = 360 ;
	static signed int lifeRemaining = kLifetime ;

	UIPanel * resultsPanel = rcast< UIPanel * >( GetScreen( "Gameplay" )->GetElement( kUI_GPResultsPanelName ) ) ;

	  // Show panel if this is the first run of this function this game
	if ( GetSession()->GetMenu() == 0 )
	{
		UIEditText * winLose = scast< UIEditText * >( resultsPanel->GetElement( kUI_GPResultsWinLoseName ) ) ;
		UIEditText * winner  = scast< UIEditText * >( resultsPanel->GetElement( kUI_GPResultsWinnerName ) ) ;

		  // Update whether the player won or lost
		if ( GetSession()->GetRules()->Winner() == GetMyTurn() )
			winLose->SetCaption( kUI_GPResultsWinCap ) ;
		else
			winLose->SetCaption( kUI_GPResultsLoseCap ) ;

		  // Update the player name of the winner
		eGameType gameType = GetSession()->GetRules()->GetGameType() ;
		ASSERT( gameType >= 0 && gameType < GAME_TYPE_COUNT ) ;
        winner->SetCaption( GetSession()->GetPlayer( GetSession()->GetRules()->Winner() )->GetName() + kUI_GPResultsPlayerCap[ gameType ] ) ;

		  // Display results panel
		GetSession()->ShowMenu( kUI_GPResultsPanelName ) ;
	}

	  // Only advance counter after the panel is set on the screen
	if ( resultsPanel->IsInTransition() || resultsPanel->IsDead() )
		lifeRemaining = kLifetime ;

	  // Advance the counter
	lifeRemaining-- ;
	if ( lifeRemaining <= 0 )
	{
		  // End the game
		GetSession()->HideMenu() ;
		TransitionTo( GetMainMenuStateID() ) ;
	}
}

/*  ________________________________________________________________________ */
void SplashKeyInt(int /*key*/)
/*!
*/
{ 
  if(Game::Get()->CurrentState() == Game::Get()->splash1SID)
  {
    Game::Get()->TransitionTo(Game::Get()->splash2SID);
    return;
  }
  if(Game::Get()->CurrentState() == Game::Get()->splash2SID)
  {
    Game::Get()->TransitionTo(Game::Get()->GetMainMenuStateID()); 
    return;
  }
}

/*  ________________________________________________________________________ */
void SplashClickInt(int /*x*/,int /*y*/,int /*z*/)
/*! 
*/
{
  if(Game::Get()->CurrentState() == Game::Get()->splash1SID)
  {
    Game::Get()->TransitionTo(Game::Get()->splash2SID);
    return;
  }
  if(Game::Get()->CurrentState() == Game::Get()->splash2SID)
  {
    Game::Get()->TransitionTo(Game::Get()->GetMainMenuStateID()); 
    return;
  }
}
