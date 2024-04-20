/* =========================================================================
   
    @file    GameInit.h
    @author  jmc
    @brief   Implementation of game init functions.
    
   ========================================================================= */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "DXFont.h"

#include "NetGameDiscovery.h"

#include "Input.h"
#include "DXLine.h"
#include "Ball.h"

#include "UIScreen.h"
#include "UIElement.h"
#include "UIGraphic.h"
#include "UIButton.h"
#include "UIEditText.h"
#include "UIPanel.h"
#include "UIPowerMeter.h"
#include "UIListbox.h"
#include "UIMenu.h"

#include "GraphicsRenderer.h"

#include "CollisionEngine.h"
#include "AIPlayer.h"
#include "RuleSystem.h"
#include "Particles.h"
#include "shotprojection.h"


/*                                                                prototypes
---------------------------------------------------------------------------- */

namespace
{
  // options screen tab helper
  void InsertOptionsTabs(UIPanel *p);
  
  // menu panel helpers
  UIPanel* BuildMenuPanel(void);
  UIPanel* BuildHelpPanel(void);
  UIPanel* BuildResultsPanel(void);
  UIPanel* BuildStatusPanel(void);
  UIPanel* BuildScoreboardPanel(void);
  UIPanel* BuildJukeboxPanel(void);

  // text loader
  void LoadTextChunks(void);
}


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  // text chunks
  std::string  gCreditsText;       //!< From data/text/credits.txt
  std::string  gHelpText;          //!< From data/text/help.txt
  std::string  gGameTypeDescText[ GAME_TYPE_COUNT ] ;  //!< From data/text/18-Ball.txt,
                                                       //   from data/text/19-Ball.txt,
                                                       //   from data/text/StraightPool.txt,
                                                       //   from data/text/8-11.txt
  
  UIElement::Callback  gPButtonKickCB[] = { UI_GOP0ButtonKickClick,
                                            UI_GOP1ButtonKickClick,
                                            UI_GOP2ButtonKickClick,
                                            UI_GOP3ButtonKickClick,
                                            UI_GOP4ButtonKickClick,
                                            UI_GOP5ButtonKickClick,
                                            UI_GOP6ButtonKickClick,
                                            UI_GOP7ButtonKickClick };
  UIElement::Callback  gPButtonAddAICB[] = { UI_GOP0ButtonAddAIClick,
                                             UI_GOP1ButtonAddAIClick,
                                             UI_GOP2ButtonAddAIClick,
                                             UI_GOP3ButtonAddAIClick,
                                             UI_GOP4ButtonAddAIClick,
                                             UI_GOP5ButtonAddAIClick,
                                             UI_GOP6ButtonAddAIClick,
                                             UI_GOP7ButtonAddAIClick };
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
void Game::Init(void)
/*! Game startup function.
*/
{
  // Load text data.
  LoadTextChunks();

  // Create camera and input/viewing related objects.
  mCamera = new Camera;
	mInput  = new Input;

  // Get the time of day.   
time_t  now;
tm     *curTime;

  ::time(&now);
  curTime = ::localtime(&now);
  
	// Create skybox based on time of day. The skybox needs to be created
	// before anything else that is renderable in any way, since it stores
	// the sun and ambient color data.
	mSkybox = new Skybox(mWindow->GetRenderer(),curTime->tm_hour);
 
  // Create FPS, message renderers.
  mFPSFont = new DXFont(mWindow->GetRenderer(),"Tahoma",kUIElem_DefaultTextSize,0);
  mMsgFont = new DXFont(mWindow->GetRenderer(),"Tahoma",kUIElem_DefaultTextSize,0);
	
  // Create physics engines. We need to read some constants from the playfield section
  // of our internal INI file, since they govern the size of the field.
int    pw     = ::GetPrivateProfileInt("Playfield","Width",kPlayfieldDefW,"data/config/internal.ini");
int    ph     = ::GetPrivateProfileInt("Playfield","Height",kPlayfieldDefH,"data/config/internal.ini");
int    pd     = ::GetPrivateProfileInt("Playfield","Depth",kPlayfieldDefD,"data/config/internal.ini");
int    rack   = ::GetPrivateProfileInt("Playfield","InitialRack",EIGHTEEN_BALL,"data/config/internal.ini");
//float  half_w = static_cast< float >(pw) / 2.0f;
//float  half_h = static_cast< float >(ph) / 2.0f;
//float  half_d = static_cast< float >(pd) / 2.0f;

  mPhysics   = new Physics::Engine(/*Geometry::Box3D(Geometry::Point3D(-half_w,-half_h,-half_d),Geometry::Point3D(half_w,half_h,half_d))*/);
  mAIPhysics = new Physics::Engine(/*Geometry::Box3D(Geometry::Point3D(-half_w,-half_h,-half_d),Geometry::Point3D(half_w,half_h,half_d))*/);
  mSound     = new SoundManager;  
  mAim       = new ShotProject;

  /*int i;
  i = mSound->Load2DObject("jules.mp3");
  mSound->PlayObject(i);*/

  // Create AI player.
  mAIPlayer = new AIPlayer("data/AIPlayer.bpn",1);
  mAIPlayer->SetName( kUI_GOSlotAIBtnCap ) ;

  // Construct the playfield based on INI file data.
  mPlayfield = new Playfield(static_cast< float >(pw),static_cast< float >(ph),static_cast< float >(pd),mWindow->GetRenderer(),mPhysics);
  mPlayfield->LoadBalls(static_cast< eGameType >(rack));
  mPlayfield->RackBalls(static_cast< eGameType >(rack));

  //mRules = new Rules_EighteenBall();
  //
  //mParticles = new Particles< DXSphere, particle_rule, kParticleCount >(mWindow->GetRenderer());
  ////mParticles->Origin(D3DXVECTOR3());
  //mParticles->Init();

	// Hook up message handlers.
  mWindow->InstallCallback(WM_SYSCOMMAND,Callback_WM_SYSCOMMAND);

  mWindow->InstallCallback(GM_NETSERVER_ACCEPT,Callback_GM_NETSERVER_ACCEPT);
  mWindow->InstallCallback(GM_NETSERVER_READ,Callback_GM_NETSERVER_READ);
  mWindow->InstallCallback(GM_NETCLIENT_CONNECT,Callback_GM_NETCLIENT_READ);

  // Setup WinSock.
WSADATA  wsaData;

  ENFORCE(0 == ::WSAStartup(MAKEWORD(1,1),&wsaData))
         ("Failed to initialize network interface.");
	
	// Initialize physics engine.
	mPhysics->SetGravity(Geometry::Vector3D(0.0f,0.0f,0.0f));
	mPhysics->SetMinTimeStep(1.0f / 1000.0f);

    
  // Set up physics callbacks.
  mPhysics->AddPhysicsCallback(kCallbackGoneStatic,Physics_OnStaticCB);
  mPhysics->AddPhysicsCallback(kCollisionCBSpherePlane,Physics_OnPlaneContactCB);
  mPhysics->AddPhysicsCallback(kCollisionCBSpherePocket,Physics_OnPocketContactCB);
  mPhysics->AddPhysicsCallback(kCollisionCBSphereSphere,Physics_OnSphereContactCB);
  mPhysics->AddPhysicsCallback(kCallbackRuleSS,RuleCollisionSS_CB);
  mPhysics->AddPhysicsCallback(kCallbackRuleSP,RuleCollisionSP_CB);
  mPhysics->AddPhysicsCallback(kCallbackRuleSBP,RuleCollisionSBP_CB);

	// Set up states.
	splash1SID          = DefineState(GameState_Splash1Update,GameState_Splash1Enter,GameState_Splash1Exit);
	splash2SID          = DefineState(GameState_Splash2Update,GameState_Splash2Enter,GameState_Splash2Exit);
	mMainMenuStateID    = DefineState(GameState_MainMenuUpdate,GameState_MainMenuEnter,GameState_MainMenuExit);
	mGameSelectStateID  = DefineState(GameState_GameSelectUpdate,GameState_GameSelectEnter,GameState_GameSelectExit);
	mGameOptionsStateID = DefineState(GameState_GameOptionsUpdate,GameState_GameOptionsEnter,GameState_GameOptionsExit);
	mGameplayStateID    = DefineState(GameState_GameplayUpdate,GameState_GameplayEnter,GameState_GameplayExit);
	mOptionsStateID     = DefineState(GameState_OptionsUpdate,GameState_OptionsEnter,GameState_OptionsExit);
	mCreditsStateID     = DefineState(GameState_CreditsUpdate,GameState_CreditsEnter,GameState_CreditsExit);
	
	// Initialize UI screens.
	nInitMMUI();
	nInitGSUI();
	nInitGOUI();
	nInitGPUI();
	nInitOptionsVideoUI();
	nInitCreditsUI();
	
    // Set up jukebox info
  std::vector< std::string > songNames ;
  GetJukeboxSongs( songNames ) ;
  for ( unsigned int i = 0 ; i < songNames.size() ; ++i )
  {
	  JBInfo newSong ;
	  newSong.m_songID = -1 ;
	  newSong.m_songFilename = songNames[ i ] ;
	  m_jukeboxSongs.push_back( newSong ) ;
  }

	// Start in the main menu state.
	TransitionTo(splash1SID);
}

/*  ________________________________________________________________________ */
void Game::nInitMMUI(void)
/*! Initialize the main menu UI screen.
*/
{
UIElement *e;
int        w = GetWindow()->Width();
int        h = GetWindow()->Height();

  SetActiveScreen("Splash1");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  {
    // The image.
    e = new UIGraphic(w / 2 - 256,h / 2 - 128,512,256,"data/misc/dit.png");
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kNoTrans,kUI_TransSz);
    GetScreen()->AddElement(e);
  }
  
  SetActiveScreen("Splash2");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  {
    // The image.
    e = new UIGraphic(w / 2 - 256,h / 2 - 128,512,256,"data/misc/sns.png");
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kNoTrans,kUI_TransSz);
    GetScreen()->AddElement(e);
  }

  SetActiveScreen("MainMenu");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  {
    // The title (in four parts).
    e = new UIGraphic(w - (2 * kUI_TitleW),kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_Title1Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - kUI_TitleW,kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_Title2Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - kUI_TitleW,kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_Title3Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromRight,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - kUI_TitleW,kUI_DistFromEdge + kUI_TitleH,kUI_TitleW,kUI_TitleH,kUI_Title4Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromRight,kUI_TransSz);
    GetScreen()->AddElement(e);
     
    // "Play Game" button.
    e = new UIButton(kUI_DistFromEdge,h - 3 * (kUI_DistFromEdge + kUI_MMButtonH),kUI_MMButtonW,kUI_MMButtonH,kUI_PlayBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_MMPlayClick);
    e->SetTransitionProps(kUI_PlayBtnStall,kUI_PlayBtnSpeed,UIElement::kSlideFromLeft);
    GetScreen()->AddElement(e);
    
    // The "Options" button.
    //e = new UIButton(kUI_DistFromEdge,h - 3 * (kUI_DistFromEdge + kUI_MMButtonH),kUI_MMButtonW,kUI_MMButtonH,kUI_OptionsBtnCap);
    //e->InstallCallback(UIElement::kLeftClick,UI_MMOptionsClick);
    //e->SetTransitionProps(kUI_OptionsBtnStall,kUI_OptionsBtnSpeed,UIElement::kSlideFromLeft);
    //e->Enable(false);
    //GetScreen()->AddElement(e);
    
    // The "Credits" button.
    e = new UIButton(kUI_DistFromEdge,h - 2 * (kUI_DistFromEdge + kUI_MMButtonH),kUI_MMButtonW,kUI_MMButtonH,kUI_CreditsBtnCap);
	e->InstallCallback(UIElement::kLeftClick,UI_MMCreditsClick);
    e->SetTransitionProps(kUI_CreditsBtnStall,kUI_CreditsBtnSpeed,UIElement::kSlideFromLeft);
    GetScreen()->AddElement(e);
    
    // The "Quit" button.
    e = new UIButton(kUI_DistFromEdge,h - 1 * (kUI_DistFromEdge + kUI_MMButtonH),kUI_MMButtonW,kUI_MMButtonH,kUI_QuitBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_MMQuitClick);
    e->SetTransitionProps(kUI_QuitBtnStall,kUI_QuitBtnSpeed,UIElement::kSlideFromLeft);
    GetScreen()->AddElement(e);
  }
}

/*  ________________________________________________________________________ */
void Game::nInitGSUI(void)
/*! Initialize the game select UI screen.
*/
{
UIElement *e = 0;
UIPanel   *p = 0;
int        w = GetWindow()->Width();
int        h = GetWindow()->Height();

  SetActiveScreen("GameSelect");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  { 
    // The title (in three parts).
    e = new UIGraphic(w - (3 * kUI_TitleW),kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_GS1Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - (2 * kUI_TitleW),kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_GS2Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - kUI_TitleW,kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_GS3Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    
    // The panel.
    p = new UIPanel(w / 2 - kUI_MPanelW / 2,h / 2 - kUI_GSPanelH / 2,kUI_MPanelW,kUI_GSPanelH);
    p->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom);

    // Player name.
    char  nameBuffer[256];
    DWORD nameLen = 256;
    std::string  nameStr = "Player";
    
    if(::GetUserName(nameBuffer,&nameLen))
      nameStr = nameBuffer;

    e = new UIEditText(kUI_DistFromEdge,kUI_DistFromEdge,kUI_GSListW,kUI_EditBoxH,nameStr);
    e->SetCaption("Your Name:");
    e->SideCaption(true);
	e->Width( kUI_GSListW - e->GetCaptionOffset().cx ) ;
	static_cast< UIEditText* >(e)->MaxSize( 40 ) ;
    static_cast< UIEditText* >(e)->SingleLine(true);
    p->AddElement(kUI_GSPlayerName,e);
    
    // Game list.
    e = new UIListbox(kUI_DistFromEdge,40,kUI_GSListW,kUI_GSListH);
    e->SetCaption("Available Games:");
    p->AddElement(kUI_GSListName,e);

      // Help Tip
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW , kUI_DistFromEdge ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        2 * kUI_EditBoxH , kUI_GSGameHelpText ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    //scast< UIEditText * >( e )->SingleLine( true ) ;
    scast< UIEditText * >( e )->Centered( true ) ;
    e->Enable( false ) ;
    p->AddElement( e ) ;

      // Name of currently selected game
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW , kUI_GSGameListX + kUI_EditBoxH ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        2 * kUI_EditBoxH , "" ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    scast< UIEditText * >( e )->SingleLine( true ) ;
    scast< UIEditText * >( e )->Centered( true ) ;
    e->Enable( false ) ;
    p->AddElement( kUI_GSSelGameName , e ) ;

      // Game Type of currently selected game
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW ,
                        kUI_GSGameListX + 2 * kUI_DistFromEdge + 2 * kUI_EditBoxH ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        kUI_EditBoxH , "" ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    scast< UIEditText * >( e )->SingleLine( true ) ;
    e->SetCaption( kUI_GSGameTypeCap ) ;
    e->SideCaption( true ) ;
    e->Enable( false ) ;
    p->AddElement( kUI_GSSelGameType , e ) ;

      // Creator (host player name) of currently selected game
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW ,
                        kUI_GSGameListX + 2 * kUI_DistFromEdge + 3 * kUI_EditBoxH ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        kUI_EditBoxH , "" ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    scast< UIEditText * >( e )->SingleLine( true ) ;
    e->SetCaption( kUI_GSGameHostCap ) ;
    e->SideCaption( true ) ;
    e->Enable( false ) ;
    p->AddElement( kUI_GSSelGameHost , e ) ;

      // Number of human players in currently selected game
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW ,
                        kUI_GSGameListX + 3 * kUI_DistFromEdge + 4 * kUI_EditBoxH ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        kUI_EditBoxH , "" ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    scast< UIEditText * >( e )->SingleLine( true ) ;
    e->SetCaption( kUI_GSGameHumanPCap ) ;
    e->SideCaption( true ) ;
    e->Enable( false ) ;
    p->AddElement( kUI_GSSelGameHumanP , e ) ;

      // Number of AI players in currently selected game
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW ,
                        kUI_GSGameListX + 3 * kUI_DistFromEdge + 5 * kUI_EditBoxH ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        kUI_EditBoxH , "" ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    scast< UIEditText * >( e )->SingleLine( true ) ;
    e->SetCaption( kUI_GSGameAIPCap ) ;
    e->SideCaption( true ) ;
    e->Enable( false ) ;
    p->AddElement( kUI_GSSelGameAIP , e ) ;

      // Number of available slots in currently selected game
    e = new UIEditText( 2 * kUI_DistFromEdge + kUI_GSListW ,
                        kUI_GSGameListX + 3 * kUI_DistFromEdge + 6 * kUI_EditBoxH ,
                        kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_GSListW ,
                        kUI_EditBoxH , "" ) ;
    scast< UIEditText * >( e )->ReadOnly( true ) ;
    scast< UIEditText * >( e )->SingleLine( true ) ;
    e->Enable( false ) ;
    p->AddElement( kUI_GSSelGameAvailP , e ) ;

    // Add the panel.
    GetScreen()->AddElement(kUI_GSPanelName,p);
    
    // The "Create Game" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2,
                     h / 2 - kUI_GSPanelH / 2 + (kUI_GSPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_GSCreateBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_GSCreateGameClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(e);
    
    // The "Join Game" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2 + (kUI_MMButtonW + kUI_DistFromEdge),
                     h / 2 - kUI_GSPanelH / 2 + (kUI_GSPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_GSJoinBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_GSJoinGameClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(kUI_GSJoinButton,e);
    
    // The "Back" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2 + (kUI_MPanelW - kUI_MMButtonW),
                     h / 2 - kUI_GSPanelH / 2 + (kUI_GSPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_BackBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_GSBackClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(e);
  }
}

/*  ________________________________________________________________________ */
void Game::nInitGOUI(void)
/*! Initialize the game options UI screen.
*/
{
UIElement *e;
UIPanel   *p;
int        w = GetWindow()->Width();
int        h = GetWindow()->Height();

  SetActiveScreen("GameOptions");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  { 
    // The title (in three parts).
    e = new UIGraphic(w - (3 * kUI_TitleW),kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_GO1Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - (2 * kUI_TitleW),kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_GO2Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    e = new UIGraphic(w - kUI_TitleW,kUI_DistFromEdge,kUI_TitleW,kUI_TitleH,kUI_GO3Str);
    e->SetTransitionProps(kUI_TitleStall,kUI_TitleSpeed,UIElement::kSlideFromTop,kUI_TransSz);
    GetScreen()->AddElement(e);
    
    // The "Start Game" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2,
                     h / 2 - kUI_MPanelH / 2 + (kUI_MPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_GOStartBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_GOStartClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(kUI_GOStartBtnCap,e);
    
    // The "Back" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2 + (kUI_MPanelW - kUI_MMButtonW),
                     h / 2 - kUI_MPanelH / 2 + (kUI_MPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_BackBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_GOBackClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(e);
    
    // The panel.
    p = new UIPanel(w / 2 - kUI_MPanelW / 2,h / 2 - kUI_MPanelH / 2,kUI_MPanelW,kUI_MPanelH);
    p->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom);
    
    // The game name edit box.
    e = new UIEditText(kUI_DistFromEdge,kUI_DistFromEdge,200,kUI_EditBoxH,"Game Title");
    e->SetCaption("Game Name:");
    e->SideCaption(true);
    e->InstallCallback(UIElement::kLoseFocus,UI_GONameLostFocus);
	scast< UIEditText * >(e)->MaxSize( 40 ) ;
    static_cast< UIEditText* >(e)->SingleLine(true);
    p->AddElement(kUI_GOTitleName,e);

    // The players list captions.
    e = new UIEditText(kUI_DistFromEdge,
                       2 * kUI_DistFromEdge + kUI_EditBoxH,
                       200,
                       kUI_EditBoxH,
                       "Players:");
    e->Enable(false);
    p->AddElement(e);
    
    for(unsigned int i = 0; i < kPlayersMax; ++i)
    {
      e = new UIEditText(kUI_DistFromEdge + kUI_GOPlayersListX,
                         ((2 + i) * kUI_DistFromEdge + (2 + i) * kUI_EditBoxH),
                         kUI_GOPlayersNumW,kUI_EditBoxH,lexical_cast< std::string >(i + 1));
      static_cast< UIEditText* >(e)->SingleLine(true);
      e->Enable(false);
      p->AddElement(e);
    }
    
    // The players list buttons.
    for(unsigned int i = 0; i < kPlayersMax; ++i)
    {
      // Name.
      e = new UIButton(kUI_DistFromEdge + kUI_GOPlayersListX + kUI_GOPlayersNumW,
                       ((2 + i) * kUI_DistFromEdge + (2 + i) * kUI_EditBoxH),
                       kUI_GOPlayersListW,kUI_EditBoxH,kUI_GOSlotAvailBtnCap);
      p->AddElement(kUI_GOPButtonName[i],e);
      
      // Kick/close/open.
      e = new UIButton(2 * kUI_DistFromEdge + kUI_GOPlayersListX + kUI_GOPlayersNumW + kUI_GOPlayersListW,
                       ((2 + i) * kUI_DistFromEdge + (2 + i) * kUI_EditBoxH),
                       kUI_GOPlayersKickW,kUI_EditBoxH,kUI_GOCloseBtnCap,12);
      e->InstallCallback(UIElement::kLeftClick,gPButtonKickCB[i]);
	  if ( i == 0 )
		  e->Enable( false ) ;  // host
      p->AddElement(kUI_GOPKickButtonName[i],e);
      
      // Add AI.
      e = new UIButton(3 * kUI_DistFromEdge + kUI_GOPlayersListX + kUI_GOPlayersNumW + kUI_GOPlayersListW + kUI_GOPlayersKickW,
                       ((2 + i) * kUI_DistFromEdge + (2 + i) * kUI_EditBoxH),
                       kUI_GOPlayersAddW,kUI_EditBoxH,kUI_GOAddAIBtnCap,12);
      e->InstallCallback(UIElement::kLeftClick,gPButtonAddAICB[i]);
	  if ( i == 0 )
		  e->Enable( false ) ;  // host
      p->AddElement(kUI_GOPAddAIButtonName[i],e);
    }

    // The game type menu.
    e = new UIMenu(kUI_GOTypeX,
                   kUI_DistFromEdge,
                   0,//kUI_GOTypeW,
                   0);//kUI_EditBoxH);
    e->SideCaption(true);
    e->SetCaption("Game Type:");
    static_cast< UIMenu* >(e)->AddItem("18-ball");
    static_cast< UIMenu* >(e)->AddItem("19-ball");
    static_cast< UIMenu* >(e)->AddItem("Cutthroat");
    p->AddElement(kUI_GOTypeMenuName,e);

	  // The game type radio buttons
	for ( int i = 0 ; i < GAME_TYPE_COUNT ; ++i )
	{
		  // Game Type Radio Button
		e = new UIButton( kUI_GOTypeX + kUI_RadioBtnW , ( i + 2 ) * kUI_DistFromEdge + ( i + 1 ) * kUI_EditBoxH ,
		                  kUI_MPanelW - kUI_GOTypeX - kUI_DistFromEdge - kUI_RadioBtnW , kUI_EditBoxH ,
		                  kUI_GOGameTypeBtnCap[ i ] ) ;
		e->InstallCallback( UIElement::kLeftClick , UI_GOGameTypeClick[ i ] ) ;
		p->AddElement( kUI_GOGameTypeBtnName[ i ] , e ) ;

		  // Image for the radio button
		if ( i == 0 )
		{
			e = new UIGraphic( kUI_GOTypeX , ( i + 2 ) * kUI_DistFromEdge + ( i + 1 ) * kUI_EditBoxH ,
			                   kUI_RadioBtnW , kUI_RadioBtnH , kUI_RadioBtnOnStr ) ;
		}
		else
		{
			e = new UIGraphic( kUI_GOTypeX , ( i + 2 ) * kUI_DistFromEdge + ( i + 1 ) * kUI_EditBoxH ,
			                   kUI_RadioBtnW , kUI_RadioBtnH , kUI_RadioBtnOffStr ) ;
		}
		p->AddElement( kUI_GOGameTypeImgName[ i ] , e ) ;

		  // Description for the selected game type
		UIPanel * descP = new UIPanel( p->X() + kUI_GOTypeX , p->Y() + (GAME_TYPE_COUNT + 2) * kUI_DistFromEdge + (GAME_TYPE_COUNT + 1) * kUI_EditBoxH ,
		                               kUI_MPanelW - kUI_GOTypeX - kUI_DistFromEdge ,
		                               (kPlayersMax - GAME_TYPE_COUNT + 2) * kUI_DistFromEdge + (kPlayersMax - GAME_TYPE_COUNT) * kUI_EditBoxH ) ;
		descP->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIPanel::kSlideFromRight ) ; // woosh
		if ( i == 0 )
		{
			descP->Die( false ) ;
			descP->Reset() ;
		}
		else
		{
			descP->Exit() ;
			descP->Die( true ) ;
		}
		e = new UIEditText( 0 , 0 , kUI_MPanelW - kUI_GOTypeX - kUI_DistFromEdge ,
		                    (kPlayersMax - GAME_TYPE_COUNT + 2) * kUI_DistFromEdge + (kPlayersMax - GAME_TYPE_COUNT) * kUI_EditBoxH ,
		                    gGameTypeDescText[ i ] ) ;
		static_cast< UIEditText * >( e )->ReadOnly( true );
		descP->AddElement( kUI_GOGameTypeDscName[ i ] , e ) ;
		GetScreen()->AddElement( kUI_GOGameTypePnlName[ i ] , descP ) ;
	}

      // Chat Display UI
    e = new UIListbox( kUI_DistFromEdge , (3 + kPlayersMax) * kUI_DistFromEdge + (1 + kPlayersMax) * kUI_EditBoxH ,
	                   kUI_MPanelW - 2 * kUI_DistFromEdge , kUI_MPanelH - ( (5 + kPlayersMax) * kUI_DistFromEdge + (2 + kPlayersMax) * kUI_EditBoxH ) - kUI_EditBoxH ) ;
    p->AddElement( kUI_GOChatDisplayName , e ) ;

      // Chat Text Entry UI
    e = new UIEditText( kUI_DistFromEdge , kUI_MPanelH - kUI_DistFromEdge - kUI_EditBoxH ,
		                kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_MMButtonW ,
						kUI_EditBoxH , "" ) ;
    e->SetCaption( "Chat Entry:" ) ;
    e->SideCaption( true ) ;
    e->InstallCallback(UIElement::kLoseFocus,UI_GOChatLostFocus);
	e->Width( kUI_MPanelW - 3 * kUI_DistFromEdge - kUI_MMButtonW - e->GetCaptionOffset().cx ) ;
	scast< UIEditText * >(e)->MaxSize( 100 ) ;
    static_cast< UIEditText * >( e )->SingleLine( true ) ;
    p->AddElement( kUI_GOChatEntryName , e ) ;

      // Chat Send Button
    e = new UIButton( kUI_MPanelW - kUI_DistFromEdge - kUI_MMButtonW ,
                      kUI_MPanelH - kUI_DistFromEdge - kUI_EditBoxH ,
                      kUI_MMButtonW , kUI_EditBoxH , kUI_GOChatSendBtnCap ) ;
    e->InstallCallback( UIElement::kLeftClick , UI_GOSendChatClick ) ;
    p->AddElement( kUI_GOChatSendButtonName , e ) ;

    // Add the panel.
    GetScreen()->AddElement(kUI_GOPanelName,p);
  }
}

/*  ________________________________________________________________________ */
void Game::nInitGPUI(void)
/*! Initialize the gameplay UI screen.
*/
{
UIElement *e = 0;
UIPanel   *p = 0;
int        w = GetWindow()->Width();
int        h = GetWindow()->Height();

  SetActiveScreen("Gameplay");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  {
    // The power meter.
    e = new UIPowerMeter(kUI_DistFromEdge,50);
    e->SetTransitionProps(0.0,1.0f,UIElement::kSlideFromLeft);
    e->Die(true);
    GetScreen()->AddElement(kUI_GPPowerMeterName,e);
    
    // The chat box.
    e = new UIEditText(kUI_DistFromEdge,h - (kUI_EditBoxH + kUI_DistFromEdge),kUI_GPChatBoxW,kUI_EditBoxH,"");
    e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromBottom);
    e->SetCaption("Say:");
    e->SideCaption(true);
	scast< UIEditText * >(e)->MaxSize( 100 ) ;
	scast< UIEditText * >(e)->SingleLine( true ) ;
    e->Die(true);
    GetScreen()->AddElement(kUI_GPChatBoxName,e);

    // The status bar.
    p = BuildStatusPanel();
    GetScreen()->AddElement(kUI_GPStatusPanelName,p);

	  // Scoreboard/Player List Toggle
	e = new UIButton( w - kUI_DistFromEdge - kUI_MMButtonH , kUI_DistFromEdge ,
		              kUI_MMButtonH , kUI_EditBoxH * 9 + kUI_DistFromEdge * 11 ,
					  kUI_GPScoreboardBtnCap ) ;
	e->InstallCallback( UIElement::kLeftClick , UI_GPSBToggleClick ) ;
	GetScreen()->AddElement( kUI_GPScoreboardToggleName , e ) ;

	  // Jukebox Toggle
	e = new UIButton( w - kUI_DistFromEdge - kUI_MMButtonH , kUI_EditBoxH * 9 + kUI_DistFromEdge * 13 ,
		              kUI_MMButtonH , kUI_EditBoxH * 5 + kUI_DistFromEdge * 6 + kUI_GPJBPlaylistH ,
					  kUI_GPJukeboxBtnCap ) ;
	e->InstallCallback( UIElement::kLeftClick , UI_GPJBToggleClick ) ;
	GetScreen()->AddElement( kUI_GPJukeboxToggleName , e ) ;

	  // Player Turn Indicator
	p = new UIPanel( w / 2 - kUI_GPTurnIndicatorW / 2 , h / 2 - kUI_GPTurnIndicatorH / 2 ,
	                 kUI_GPTurnIndicatorW , kUI_GPTurnIndicatorH ) ;
	p->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIPanel::kSlideFromBottom ) ;
	p->Die( true ) ;
	e = new UIEditText( kUI_DistFromEdge , kUI_GPTurnIndicatorH / 2 - kUI_EditBoxH / 2 ,
	                    kUI_GPTurnIndicatorW - 2 * kUI_DistFromEdge , kUI_EditBoxH , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	scast< UIEditText * >( e )->Centered( true ) ;
	p->AddElement( kUI_GPTurnIndTextName , e ) ;
	GetScreen()->AddElement( kUI_GPTurnIndicatorName , p ) ;

    // The game menu panel.
    p = BuildMenuPanel();
    GetScreen()->AddElement(kUI_GPMenuPanelName,p);

	// The help menu panel.
    p = BuildHelpPanel();
    GetScreen()->AddElement(kUI_GPHelpPanelName,p);

	  // Game end results panel
	p = BuildResultsPanel();
	GetScreen()->AddElement( kUI_GPResultsPanelName , p ) ;

	  // Scoreboard panel
	p = BuildScoreboardPanel() ;
	GetScreen()->AddElement( kUI_GPScoreboardPanelName , p ) ;

	  // Jukebox panel
	p = BuildJukeboxPanel() ;
	GetScreen()->AddElement( kUI_GPJukeboxPanelName , p ) ;
	
	  // State change messages.
	  e = new UIGraphic(2 * kUI_DistFromEdge + kUI_GPStatusBarW,
	                    kUI_DistFromEdge,
	                    256,
	                    256,kUI_GPInHandStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromRight);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPMsgBallInHandName,e);
	  e = new UIGraphic(2 * kUI_DistFromEdge + kUI_GPStatusBarW,
	                    kUI_DistFromEdge,
	                    256,
	                    256,kUI_GPLineupStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromTop);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPMsgShotLineupName,e);
	  e = new UIGraphic(2 * kUI_DistFromEdge + kUI_GPStatusBarW,
	                    kUI_DistFromEdge,
	                    256,
	                    256,kUI_GPSetPowerStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromTop);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPMsgSetPowerName,e);
	  e = new UIGraphic(2 * kUI_DistFromEdge + kUI_GPStatusBarW,
	                    kUI_DistFromEdge,
	                    256,
	                    256,kUI_GPCallShotStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromTop);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPMsgCallShotName,e);
	/*  e = new UIGraphic(2 * kUI_DistFromEdge + kUI_GPStatusBarW,
	                    kUI_DistFromEdge,
	                    256,
	                    256,kUI_GPYourTurnStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromRight);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPMsgYourTurnName,e);*/
	  
	  // Camera/cue/etc indicators.
	  e = new UIGraphic(w - (2 * kUI_DistFromEdge) - (2 * kUI_GPIndW),
	                    h - kUI_DistFromEdge - kUI_GPIndH,
	                    kUI_GPIndW,
	                    kUI_GPIndH,kUI_GPCamIndStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromBottom);
	  e->Die(false);
	  GetScreen()->AddElement(kUI_GPCamIndName,e);
	  e = new UIGraphic(w - kUI_DistFromEdge - kUI_GPIndW,
	                    h - kUI_DistFromEdge - kUI_GPIndH,
	                    kUI_GPIndW,
	                    kUI_GPIndH,kUI_GPCueIndStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromBottom);
	  e->Die(false);
	  GetScreen()->AddElement(kUI_GPCueIndName,e);
	  
	  // And the locks for the indicators.
	  e = new UIGraphic(w - (2 * kUI_DistFromEdge) - (2 * kUI_GPIndW) + (kUI_GPIndW / 2),
	                    h - kUI_DistFromEdge - kUI_GPIndH + (kUI_GPIndH / 2),
	                    kUI_GPIndW / 2,
	                    kUI_GPIndH / 2,kUI_GPLockIndStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromBottom);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPCamLockName,e);
	  e = new UIGraphic(w - kUI_DistFromEdge - kUI_GPIndW + (kUI_GPIndW / 2),
	                    h - kUI_DistFromEdge - kUI_GPIndH + (kUI_GPIndH / 2),
	                    kUI_GPIndW / 2,
	                    kUI_GPIndH / 2,kUI_GPLockIndStr);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromBottom);
	  e->Die(true);
	  GetScreen()->AddElement(kUI_GPCueLockName,e);
	  
	  // Tutor window.
	  e = new UIEditText(kUI_DistFromEdge,h - 2 * kUI_DistFromEdge - kUI_EditBoxH - kUI_GPTutorH,
	                     kUI_GPTutorW,kUI_GPTutorH,
	                     "Help text will go here later. ");
	  scast< UIEditText * >( e )->ReadOnly( true ) ;
	  e->Die(true);
	  e->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromLeft);
	  GetScreen()->AddElement(kUI_GPTutorName,e);
  }
}

/*  ________________________________________________________________________ */
void Game::nInitOptionsVideoUI(void)
/*! Initialize the video tab of the options UI screen.
*/
{
UIElement *e = 0;
UIPanel   *p = 0;
int        w = GetWindow()->Width();
int        h = GetWindow()->Height();

  SetActiveScreen("OptionsVideo");
  GetScreen()->Reset();
  if(0 == GetScreen()->NumElements())
  {
    // The "Save" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2,
                     h / 2 - kUI_MPanelH / 2 + (kUI_MPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_OptsSaveBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_OptsSaveClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(e);
    
    // The "Back" button.
    e = new UIButton(w / 2 - kUI_MPanelW / 2 + (kUI_MMButtonW + kUI_DistFromEdge),
                     h / 2 - kUI_MPanelH / 2 + (kUI_MPanelH + kUI_DistFromEdge),
                     kUI_MMButtonW,kUI_MMButtonH,kUI_OptsBackBtnCap);
    e->InstallCallback(UIElement::kLeftClick,UI_OptsBackClick);
    e->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom,kUI_MPanelH);
    GetScreen()->AddElement(e);
  
    // The panel.
    p = new UIPanel(w / 2 - kUI_MPanelW / 2,h / 2 - kUI_MPanelH / 2,kUI_MPanelW,kUI_MPanelH);
    p->SetTransitionProps(kUI_MPanelStall,kUI_MPanelSpeed,UIElement::kSlideFromBottom);
    
    // Insert the tabs.
    InsertOptionsTabs(p);

    // The resolution listbox.
    e = new UIListbox(kUI_DistFromEdge,2 * kUI_DistFromEdge + 32,200,200);
    e->SetCaption("Resolution:");

Graphics::Renderer::DisplayModeIter  it = Game::Get()->GetWindow()->GetRenderer()->ModesBegin();
UIListbox                           *lb = static_cast< UIListbox* >(e);

    while(it != Game::Get()->GetWindow()->GetRenderer()->ModesEnd())
    {
    std::stringstream  fmt;

      fmt << it->Width << "x" << it->Height << " at " << it->RefreshRate << "Hz";
      lb->AddItem(fmt.str().c_str());
      ++it;
    }
    p->AddElement(kUI_OptsVideoResListName,e);

    // Add the panel.
    GetScreen()->AddElement(kUI_OptsVideoPanelName,p);
  }
}

/*  ________________________________________________________________________ */
/*!
	\brief     Initialize game credits screen
 */
void Game::nInitCreditsUI( void )
{
	UIElement * e = 0 ;
	UIPanel   * p = 0 ;
	int         w = GetWindow()->Width() ;
	int         h = GetWindow()->Height() ;

	SetActiveScreen( "Credits" ) ;
	GetScreen()->Reset() ;

	if( 0 == GetScreen()->NumElements() )
	{
		  // The title (in two parts)
		e = new UIGraphic( w - ( 2 * kUI_TitleW ) , kUI_DistFromEdge , kUI_TitleW , kUI_TitleH , kUI_Credits1Str ) ;
		e->SetTransitionProps( kUI_TitleStall , kUI_TitleSpeed , UIElement::kSlideFromTop , kUI_TransSz ) ;
		GetScreen()->AddElement( e ) ;
		e = new UIGraphic( w - ( 1 * kUI_TitleW ) , kUI_DistFromEdge , kUI_TitleW , kUI_TitleH , kUI_Credits2Str ) ;
		e->SetTransitionProps( kUI_TitleStall , kUI_TitleSpeed , UIElement::kSlideFromTop , kUI_TransSz ) ;
		GetScreen()->AddElement( e ) ;

		  // Credits panel with credits text
		p = new UIPanel( w / 2 - kUI_MPanelW / 2 , h / 2 - kUI_MPanelH / 2 , kUI_MPanelW , kUI_MPanelH ) ;
		p->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIElement::kSlideFromBottom ) ;
		e = new UIEditText( 0 , 0 , p->Width() , p->Height() , gCreditsText ) ;
		scast< UIEditText * >( e )->Enable( false ) ;
		p->AddElement( kUI_CreditsTextName , e ) ;
		GetScreen()->AddElement( kUI_CreditsPanelName , p ) ;

		  // Button to return
		e = new UIButton( w / 2 + kUI_MPanelW / 2 - kUI_MMButtonW , h / 2 + kUI_MPanelH / 2 + kUI_DistFromEdge ,
			              kUI_MMButtonW , kUI_MMButtonH , kUI_BackBtnCap ) ;
		e->InstallCallback( UIElement::kLeftClick , UI_GSBackClick ) ;
		e->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIElement::kSlideFromBottom , kUI_MPanelH ) ;
		GetScreen()->AddElement( kUI_CreditsBackBtnName , e ) ;
	}
}


namespace
{

/*  ________________________________________________________________________ */
void InsertOptionsTabs(UIPanel *p)
/*! Insert the options screen top-row tabs into the screen.

    Do not call outside of a UI init function.

    @param p  The panel to insert into.
*/
{
UIElement *e = 0;

  // Video, audio, controls, gameplay.
  e = new UIButton(0,0,kUI_MPanelW / 4,kUI_MMButtonH,kUI_OptsVideoTabCap);
  p->AddElement(e);
  e = new UIButton(kUI_MPanelW / 4,0,kUI_MPanelW / 4,kUI_MMButtonH,kUI_OptsAudioTabCap);
  p->AddElement(e);
  e = new UIButton(2 * (kUI_MPanelW / 4),0,kUI_MPanelW / 4,kUI_MMButtonH,kUI_OptsControlsTabCap);
  p->AddElement(e);
  e = new UIButton(3 * (kUI_MPanelW / 4),0,kUI_MPanelW / 4,kUI_MMButtonH,kUI_OptsGameTabCap);
  p->AddElement(e);
}

/*  ________________________________________________________________________ */
UIPanel* BuildStatusPanel(void)
/*! Build and return a UIPanel for the in-game status area.
    
    @return
    A new UIPanel, which the caller assumes ownership of, containing the
    status area interface.
*/
{
UIPanel    *p = new UIPanel(kUI_DistFromEdge,kUI_DistFromEdge,
                          kUI_GPStatusBarW,kUI_GPStatusBarH);
UIEditText *e = 0;

  // From the top of the screen, dead until gameplay.
  p->SetTransitionProps(0.0f,1.0f,UIElement::kSlideFromTop);
  p->Die(true);
    
  // This panel doesn't render itself; only children.
  p->Logical(true);
  
  // "Shooting Now" text.
  e = new UIEditText(0,0,kUI_GPStatusBarW,kUI_GPStatusBarH,"Shooting Now:");
  e->ReadOnly( true ) ;
  p->AddElement(kUI_GPStatusTextName,e);

  return (p);
}

/*  ________________________________________________________________________ */
UIPanel* BuildMenuPanel(void)
/*! Build and return a UIPanel for the game menu.
    
    @return
    A new UIPanel, which the caller assumes ownership of, containing the
    game menu interface.
*/
{
int       w = Game::Get()->GetWindow()->Width();
int       h = Game::Get()->GetWindow()->Height();
UIPanel  *p = new UIPanel(w / 2 - kUI_GPanelW / 2,
                          h / 2 - (kUI_GPanelButtonH * 3 + kUI_DistFromEdge * 2) / 2,
                          kUI_GPanelW,
                          kUI_GPanelButtonH * 3 + kUI_DistFromEdge * 2);
UIButton *e = 0;

  // "Menu" elements come from the top of the screen.
  p->SetTransitionProps(0.0f,kUI_MPanelSpeed,UIElement::kSlideFromTop);
  
  // This panel doesn't render itself; only children.
  p->Logical(true);
  
  // Panel has buttons; resume game, quit to main menu, quit to desktop.
  e = new UIButton(0,
                   0,
                   kUI_GPanelW,
                   kUI_GPanelButtonH,
                   "Resume Game",18);
  e->InstallCallback(UIElement::kLeftClick,UI_GPMenuResumeClick);
  p->AddElement(e);
  e = new UIButton(0,
                   kUI_GPanelButtonH + kUI_DistFromEdge,
                   kUI_GPanelW,
                   kUI_GPanelButtonH,
                   "Resign Game",18);
  e->InstallCallback(UIElement::kLeftClick,UI_GPMenuResignClick);
  p->AddElement("ResignButton",e);
  e = new UIButton(0,
                   2 * (kUI_GPanelButtonH + kUI_DistFromEdge),
                   kUI_GPanelW,
                   kUI_GPanelButtonH,
                   "Quit to Desktop",18);
  e->InstallCallback(UIElement::kLeftClick,UI_GPMenuQuitClick);
  p->AddElement(e);
  
  
  // Set panel properties and return.
  p->Die(true);
  return (p);
}

/*  ________________________________________________________________________ */
UIPanel* BuildHelpPanel(void)
/*! Build and return a UIPanel for the help menu.
    
    @return
    A new UIPanel, which the caller assumes ownership of, containing the
    help menu interface.
*/
{
int         w = Game::Get()->GetWindow()->Width();
int         h = Game::Get()->GetWindow()->Height();
UIPanel    *p = new UIPanel(w / 2 - kUI_MPanelW / 2,h / 2 - kUI_MPanelH / 2,kUI_MPanelW,kUI_MPanelH);
UIEditText *e = 0;

  // "Menu" elements come from the top of the screen.
  p->SetTransitionProps(0.0f,kUI_MPanelSpeed,UIElement::kSlideFromTop);
  
  // Panel has a textbox that contains the help text.
  e = new UIEditText(0,
                     0,
                     kUI_MPanelW,
                     kUI_MPanelH - kUI_EditBoxH,
                     gHelpText.c_str());
  e->ReadOnly(true);
  p->AddElement(e);
  
  // And a smaller textbox, at the bottom, that says "press any key to continue,"
  // or something.
  e = new UIEditText(0,
                     kUI_MPanelH - kUI_EditBoxH,
                     kUI_MPanelW,
                     kUI_EditBoxH,
                     "Press F1 to return to the game.");
  e->SingleLine(true);
  e->Centered(true);
  e->ReadOnly(true);
  p->AddElement(e);
  
  // Set panel properties and return.
  p->Die(true);
  return (p);
}

/*  ________________________________________________________________________ */
/*!
\brief     Builds the game end results panel

\retval    UIPanel*    Pointer to the constructed panel
 */
UIPanel * BuildResultsPanel( void )
{
	int w = Game::Get()->GetWindow()->Width() ;
	int h = Game::Get()->GetWindow()->Height() ;

	int panelHeight = kUI_EditBoxH * 3 + kUI_DistFromEdge * 5 ;
	UIPanel *   p = new UIPanel( w / 2 - kUI_GPResultsPanelW / 2 , h / 2 - panelHeight / 2 ,
								 kUI_GPResultsPanelW , panelHeight ) ;
	UIElement * e = 0 ;

	  // Panel Setup
	p->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIPanel::kSlideFromTop ) ;
	p->Die( true ) ;

	  // Panel Title
	e = new UIEditText( kUI_DistFromEdge , kUI_DistFromEdge , 0 , kUI_EditBoxH , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	scast< UIEditText * >( e )->Centered( true ) ;
	e->SetCaption( "Game End Results" ) ;
	e->SideCaption( true ) ;
	e->X( kUI_GPResultsPanelW / 2 - e->GetCaptionOffset().cx / 2 ) ;
	p->AddElement( e ) ;

	  // "Did I win or lose?" indicator
	e = new UIEditText( kUI_DistFromEdge , kUI_EditBoxH + 3 * kUI_DistFromEdge , 0 , 0 , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	e->SetCaption( "" ) ;
	e->SideCaption( true ) ;
	p->AddElement( kUI_GPResultsWinLoseName , e ) ;

	  // Winner player name indicator
	e = new UIEditText( kUI_DistFromEdge , kUI_EditBoxH * 2 + 4 * kUI_DistFromEdge , 0 , 0 , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	e->SetCaption( "" ) ;
	e->SideCaption( true ) ;
	p->AddElement( kUI_GPResultsWinnerName , e ) ;

	return p ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Builds the scoreboard panel (in dynamically allocated memory) and returns a pointer

	\retval		UIPanel*	Pointer to the constructed panel
 */
UIPanel * BuildScoreboardPanel( void )
{
	int w = Game::Get()->GetWindow()->Width() ;

	UIPanel *   p = new UIPanel( w - 2 * kUI_DistFromEdge - kUI_MMButtonH - kUI_GPScoreboardW , kUI_DistFromEdge ,
								 kUI_GPScoreboardW , kUI_EditBoxH * 9 + kUI_DistFromEdge * 11 ) ;
	UIElement * e = 0 ;

	  // Panel Setup
	p->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIPanel::kSlideFromRight ) ;
	p->Die( true ) ;

	  // Panel Title
	e = new UIEditText( kUI_DistFromEdge , kUI_DistFromEdge , 0 , kUI_EditBoxH , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	scast< UIEditText * >( e )->Centered( true ) ;
	e->SetCaption( "Scoreboard" ) ;
	e->SideCaption( true ) ;
	e->X( kUI_GPScoreboardW / 2 - e->GetCaptionOffset().cx / 2 ) ;
	p->AddElement( kUI_GPSBTitleName , e ) ;

	  // Player Name and Score Setup
	for ( int i = 0 ; i < 8 ; ++i )
	{
		e = new UIEditText( kUI_DistFromEdge ,
			                kUI_EditBoxH + 3 * kUI_DistFromEdge + i * ( kUI_EditBoxH + kUI_DistFromEdge ) ,
							0 , 0 , "" ) ;
		scast< UIEditText * >( e )->SingleLine( true ) ;
		e->SetCaption( "(empty)" ) ;
		e->SideCaption( true ) ;
		p->AddElement( kUI_GPSBPlayerTextName[ i ] , e ) ;

		e = new UIEditText( kUI_GPScoreboardW - kUI_DistFromEdge - 100 ,
			                kUI_EditBoxH + 3 * kUI_DistFromEdge + i * ( kUI_EditBoxH + kUI_DistFromEdge ) ,
							100 , kUI_EditBoxH , "0" ) ;
		scast< UIEditText * >( e )->SingleLine( true ) ;
		scast< UIEditText * >( e )->ReadOnly( true ) ;
		p->AddElement( kUI_GPSBScoreTextName[ i ] , e ) ;
	}

	return p ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Builds the jukebox panel (in dynamically allocated memory) and returns a pointer

	\retval		UIPanel*	Pointer to the constructed panel
 */
UIPanel * BuildJukeboxPanel( void )
{
	int w = Game::Get()->GetWindow()->Width() ;

	UIPanel *   p = new UIPanel( w - 2 * kUI_DistFromEdge - kUI_MMButtonH - kUI_GPJukeboxW ,
	                             kUI_EditBoxH * 9 + kUI_DistFromEdge * 13 ,
								 kUI_GPJukeboxW ,
								 kUI_EditBoxH * 5 + kUI_DistFromEdge * 6 + kUI_GPJBPlaylistH ) ;
	UIElement * e = 0 ;

	p->SetTransitionProps( kUI_MPanelStall , kUI_MPanelSpeed , UIPanel::kSlideFromRight ) ;
	p->Die( true ) ;

	  // Panel Title
	e = new UIEditText( kUI_DistFromEdge , kUI_DistFromEdge , 0 , kUI_EditBoxH , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	scast< UIEditText * >( e )->Centered( true ) ;
	e->SetCaption( "Jukebox" ) ;
	e->SideCaption( true ) ;
	e->X( kUI_GPJukeboxW / 2 - e->GetCaptionOffset().cx / 2 ) ;
	p->AddElement( kUI_GPJBTitleName , e ) ;

	  // "Currently Playing" label
	e = new UIEditText( kUI_DistFromEdge , kUI_EditBoxH + 3 * kUI_DistFromEdge , 0 , kUI_EditBoxH , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	scast< UIEditText * >( e )->Centered( true ) ;
	e->SetCaption( "Currently Playing:" ) ;
	e->SideCaption( true ) ;
	p->AddElement( kUI_GPJBCurrentSongName , e ) ;

	  // "Currently Playing" info line
	e = new UIEditText( kUI_DistFromEdge , 2 * kUI_EditBoxH + 3 * kUI_DistFromEdge ,
	                    kUI_GPJukeboxW - 2 * kUI_DistFromEdge , kUI_EditBoxH , "" ) ;
	scast< UIEditText * >( e )->SingleLine( true ) ;
	scast< UIEditText * >( e )->ReadOnly( true ) ;
	p->AddElement( kUI_GPJBCurSongDetailsName , e ) ;

	  // Playlist
	e = new UIListbox( kUI_DistFromEdge , 3 * kUI_EditBoxH + 4 * kUI_DistFromEdge ,
	                   kUI_GPJukeboxW - 2 * kUI_DistFromEdge , kUI_GPJBPlaylistH ) ;
	e->SetCaption( "Playlist:" ) ;
	p->AddElement( kUI_GPJBPlaylistName , e ) ;

	  // Play Button
	e = new UIButton( kUI_DistFromEdge * 2 + (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 ,
	                  kUI_EditBoxH * 4 + kUI_DistFromEdge * 5 + kUI_GPJBPlaylistH ,
	                  (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 , kUI_EditBoxH , "Play" ) ;
	e->InstallCallback( UIElement::kLeftClick , UI_GPJBPlayClick ) ;
	p->AddElement( kUI_GPJBPlayButtonName , e ) ;

	  // Stop Button
	e = new UIButton( kUI_DistFromEdge * 3 + 2 * (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 ,
	                  kUI_EditBoxH * 4 + kUI_DistFromEdge * 5 + kUI_GPJBPlaylistH ,
	                  (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 , kUI_EditBoxH , "Stop" ) ;
	e->InstallCallback( UIElement::kLeftClick , UI_GPJBStopClick ) ;
	p->AddElement( kUI_GPJBStopButtonName , e ) ;

	  // Previous Button
	e = new UIButton( kUI_DistFromEdge ,
	                  kUI_EditBoxH * 4 + kUI_DistFromEdge * 5 + kUI_GPJBPlaylistH ,
	                  (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 , kUI_EditBoxH , "<< Previous" ) ;
	e->InstallCallback( UIElement::kLeftClick , UI_GPJBPrevClick ) ;
	p->AddElement( kUI_GPJBPrevButtonName , e ) ;

	  // Next Button
	e = new UIButton( kUI_DistFromEdge * 4 + 3 * (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 ,
	                  kUI_EditBoxH * 4 + kUI_DistFromEdge * 5 + kUI_GPJBPlaylistH ,
	                  (kUI_GPJukeboxW - kUI_DistFromEdge * 5) / 4 , kUI_EditBoxH , "Next >>" ) ;
	e->InstallCallback( UIElement::kLeftClick , UI_GPJBNextClick ) ;
	p->AddElement( kUI_GPJBNextButtonName , e ) ;

	return p ;
}

/*  ________________________________________________________________________ */
void LoadTextChunks(void)
/*! Load text chunks from the data files.
    
    This function reads the credits and help text files and stores them in
    globals for later access.
*/
{
  // Credits text.
  {
  std::ifstream      file("data/text/credits.txt");
  std::stringstream  fmt;
  char               buf[256]; 
  
    ENFORCE(file.is_open())
           ("Failed to find \'data/text/credits.txt\'");
    while(file)
      file.getline(buf,256),fmt << buf << "\n";
    gCreditsText = fmt.str().c_str();
  }
  
  // Help text.
  {
  std::ifstream      file("data/text/help.txt");
  std::stringstream  fmt;
  char               buf[256]; 
  
    ENFORCE(file.is_open())
            ("Failed to find \'data/text/help.txt\'");
    while(file)
      file.getline(buf,256),fmt << buf << "\n";
    gHelpText = fmt.str().c_str();
  }

  // 18-Ball description text.
  {
  std::ifstream      file("data/text/18-Ball.txt");
  std::stringstream  fmt;
  char               buf[256]; 
  
    ENFORCE(file.is_open())
            ("Failed to find \'data/text/18-Ball.txt\'");
    while(file)
      file.getline(buf,256),fmt << buf << "\n";
    gGameTypeDescText[ 0 ] = fmt.str().c_str();
  }

  // 19-Ball text.
  {
  std::ifstream      file("data/text/19-Ball.txt");
  std::stringstream  fmt;
  char               buf[256]; 
  
    ENFORCE(file.is_open())
            ("Failed to find \'data/text/19-Ball.txt\'");
    while(file)
      file.getline(buf,256),fmt << buf << "\n";
    gGameTypeDescText[ 1 ] = fmt.str().c_str();
  }

  // Straight Pool description text.
//  {
//  std::ifstream      file("data/text/StraightPool.txt");
//  std::stringstream  fmt;
//  char               buf[256]; 
//  
//    ENFORCE(file.is_open())
//            ("Failed to find \'data/text/StraightPool.txt\'");
//    while(file)
//      file.getline(buf,256),fmt << buf << "\n";
//    gGameTypeDescText[ 2 ] = fmt.str().c_str();
//  }

  // 8-11 description text.
//  {
//  std::ifstream      file("data/text/8-11.txt");
//  std::stringstream  fmt;
//  char               buf[256]; 
//  
//    ENFORCE(file.is_open())
//            ("Failed to find \'data/text/8-11.txt\'");
//    while(file)
//      file.getline(buf,256),fmt << buf << "\n";
//    gGameTypeDescText[ 3 ] = fmt.str().c_str();
//  }
  
  for(unsigned int i = 0; i < kTBBitCount; ++i)
  {
  std::stringstream  fn;
  std::stringstream  txt;
    
    fn << "data/text/tutor_" << i << ".txt";
  
  std::ifstream  file(fn.str().c_str());
  char           buf[256]; 

    while(file)
      file.getline(buf,256),txt << buf << "\n";
    Game::Get()->mTutorStrings[i] = txt.str();
  }
}

}  /* anon namespace */