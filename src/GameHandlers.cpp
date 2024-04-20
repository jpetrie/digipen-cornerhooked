/* =========================================================================
   
    @file    Game_Init.h
    @author  jmp
    @brief   Implementation of game handler and callback functions.
    
   ========================================================================= */

/*                                                                 includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "RuleSystem.h"

#include "Player.h"
#include "AIPlayer.h"

#include "NetGameDiscovery.h"
#include "NetTracker.h"
#include "NetServer.h"
#include "NetClient.h"
#include "NetPackets.h"

#include "UIScreen.h"
#include "UIElement.h"
#include "UIPanel.h"
#include "UIButton.h"
#include "UIListbox.h"
#include "UIGraphic.h"

#include "Particles.h"



/*                                                                 variables
---------------------------------------------------------------------------- */

extern int            gTransferOnExit;
extern NetServerData *gServer;


/*                                                                 functions
---------------------------------------------------------------------------- */
/*  ________________________________________________________________________ */
LRESULT CALLBACK Callback_WM_SYSCOMMAND(Window *wind,UINT msg,WPARAM wp,LPARAM lp)
/*! Invoked in response to WM_SYSCOMMAND.
    
    @param wind  The window that got the message.
    @param msg   The message code.
    @param wp    Message payload one.
    @param lp    Message payload two.
    
    @return
    A result code dependant on the individual message code.
*/
{
 ASSERT(msg == WM_SYSCOMMAND);
  
  switch(wp)
  {
    case SC_CLOSE:
      // SC_CLOSE is sent from Alt-F4 or by selecting "Close" from the system menu.
      ::PostQuitMessage(0);
      return (0);
    case SC_MAXIMIZE:
      // Maximize the window (e.g., go fullscreen).
      ::ShowWindow(wind->GetHandle(),SW_MAXIMIZE);
      return (0);
    case SC_MINIMIZE:
      // Minimize the window.
      ::ShowWindow(wind->GetHandle(),SW_MINIMIZE);
      return (0);
    case SC_RESTORE:
      ::ShowWindow(wind->GetHandle(),SW_RESTORE);
      return (0);
    default:
    {
      // Take default action.
      return (::DefWindowProc(wind->GetHandle(),msg,wp,lp));
    }
  }
}

/*  ________________________________________________________________________ */
LRESULT CALLBACK Callback_GM_NETSERVER_ACCEPT(Window * /*wind*/,UINT /*msg*/,WPARAM wp,LPARAM /*lp*/)
/*! Invoked in response to GM_NETSERVER_ACCEPT.
    
    @param wind  The window that got the message.
    @param msg   The message code.
    @param wp    Message payload one.
    @param lp    Message payload two.
    
    @return
    A result code dependant on the individual message code.
*/
{ 
  // Connection accepted, mark as pending (client
  // must send login packet to become a peer).
  extern NetServerData * gServer ;
  if ( wp == gServer->listenSock )
	NetServerAcceptPending(wp);
  return (0);
}

/*  ________________________________________________________________________ */
LRESULT CALLBACK Callback_GM_NETSERVER_READ(Window * /*wind*/,UINT /*msg*/,WPARAM wp,LPARAM /*lp*/)
/*! Invoked in response to GM_NETSERVER_READ.
    
    @param wind  The window that got the message.
    @param msg   The message code.
    @param wp    Message payload one.
    @param lp    Message payload two.
    
    @return
    A result code dependant on the individual message code.
*/
{
char  buffer[kNetPacketSz];
char  id = 0;
int   sz = 0;

  // Read the data.
  sz = recv(static_cast< SOCKET >(wp),buffer,kNetPacketSz,0);
  if(sz == 0)
  {
    // Connection was gracefully closed.
  }
  
  // Extract the packet ID.
  id = *(buffer);
  switch(id)
  {
    case PacketJoin::ID:
      NetServerHandleJoin(static_cast< SOCKET >(wp),buffer,sz);
      break;

    case PacketQuit::ID:
	{
	  unsigned int slot = kPlayersMax ;
      std::map< SOCKET , Connection >::iterator it = gServer->peerList.begin() ;
      while( it != gServer->peerList.end() )
      {
        if ( it->first == scast< SOCKET >( wp ) )
        {
          slot = it->second.id ;
          break ;
        }
        ++it ;
	  }
	  if ( slot < kPlayersMax )
	    NetServerSendKick( slot ) ;

	} break;
      
    // These packets are rebroadcast to all peers verbatim.
    case PacketTurn::ID:
    case PacketChat::ID:
    case PacketCueAdjust::ID:
      NetServerRebroadcast(buffer,sz);
      break;
    default:
    {
      // Bogus ID; should not get here.
      ASSERT(false)("Bad packet sent to server.");
    }
  }
  
  return (0);
}

/*  ________________________________________________________________________ */
LRESULT CALLBACK Callback_GM_NETCLIENT_READ(Window * /*wind*/,UINT /*msg*/,WPARAM wp,LPARAM lp)
/*! Invoked in when a client connects or when data is recieved by the client.
    
    @param wind  The window that got the message.
    @param msg   The message code.
    @param wp    Message payload one.
    @param lp    Message payload two.
    
    @return
    A result code dependant on the individual message code.
*/
{
nsl::bstream  stream;

  if(WSAGETSELECTEVENT(lp) == FD_CONNECT)
  {
    // Connection okay, send the join game information.
    NetClientSendJoin(Game::Get()->GetMyName());
  }
  else
  {
  char  buffer[kNetPacketSz];
  char  id = 0;
  int   sz = 0;

    // We must have data to read.
    sz = recv(static_cast< SOCKET >(wp),buffer,kNetPacketSz,0);
    if(sz == 0)
    {
      // Connection was gracefully closed.
      return (0);
    }
    if(sz == -1)
    {
      // Connection was beat up, tossed in the dirt, and run over by a tank.
      // And then closed. By ninjas. With sword-chucks, yo.
      return (0);
    }
    
    // Extract the packet ID.
    id = *(buffer);
    switch(id)
    {
      // Game options packet contains information about the game and who's in it.
      // We store this in the game session.
      case PacketGameOptions::ID:
      {
      PacketGameOptions  p;

        // First, unmarshall the packet.
        ASSERT(*buffer == PacketGameOptions::ID);
        stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),sz);
        stream >> id >> p.gameName >> p.playerCur >> p.playerMax;
        for(int i = 0; i < kPlayersMax; ++i)
        {
          PacketGameOptions::PlayerInfo  info;
        
          stream >> info.type >> info.name;
          p.players.push_back(info);
        }
		stream >> p.gameType ;

        // Update the session with the new information.
        Game::Get()->GetSession()->UpdateGameOptions(p);
      }
      break;
      case PacketGameStart::ID:
      {
      PacketGameStart  p;

        // First, unmarshall the packet.
        ASSERT(*buffer == PacketGameStart::ID);
        stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),sz);
        stream >> id >> p.turn;
        
        // Store local turn ID and go.
        Game::Get()->SetMyTurn(p.turn);
        Game::Get()->GetSession()->HandleStart();
      }
      break;
      case PacketTurn::ID:
      {
      PacketTurn  p;
      
        // First, unmarshall the packet.
        ASSERT(*buffer == PacketTurn::ID);
        stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),sz);
        stream >> id >> p.directionX >> p.directionY >> p.directionZ
                     >> p.power;
      
        // Then have the game handle the shot.
        Game::Get()->GetSession()->HandleShot(p.directionX,p.directionY,p.directionZ,p.power);  
      }
      break;
      case PacketEndTurnSync::ID:
      {
      PacketEndTurnSync  p; 
      std::vector< D3DXVECTOR3 > new_pos;
      std::vector< int >        pflags;
      
        ASSERT(*buffer == PacketEndTurnSync::ID);
        stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),sz);
        stream >> id >> p.ball_count;
        for(int i = 0; i < p.ball_count; ++i)
        {
        float x,y,z;
        
          stream >> x >> y >> z;
          new_pos.push_back(D3DXVECTOR3(x,y,z));
        }
        for(int i = 0; i < p.ball_count; ++i)
        {
        char fl;
        
          stream >> fl;
          pflags.push_back(fl);
          if(i > 17)
          {
            __asm nop
          }
        }
        for(unsigned int i = 0; i < new_pos.size(); ++i)
        {
        Geometry::Vector3D  zerov(0,0,0);
        Geometry::Vector3D  posv(new_pos[i].x,new_pos[i].y,new_pos[i].z);
        
          // set each ball position and velocity
          if(pflags[i])
          {
            Game::Get()->GetPlayfield()->mBalls[i]->Pocketed(true);
            Game::Get()->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[i]->ID(),Physics::Engine::propPosition,zerov);
            Game::Get()->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[i]->ID(),Physics::Engine::propVeloctity,zerov);
            //Game::Get()->GetPlayfield()->mPocketedBalls.push_back(Game::Get()->GetPlayfield()->mBalls[i]->ID());
          }
          else
          {
            Game::Get()->GetPlayfield()->mBalls[i]->Pocketed(true);
            Game::Get()->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[i]->ID(),Physics::Engine::propVeloctity,zerov);
            Game::Get()->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[i]->ID(),Physics::Engine::propPosition,posv);
          } 
        }
        
      }
      break;
      case PacketChat::ID:
      {
      PacketChat  p;
      
        // First, unmarshall the packet.
        ASSERT(*buffer == PacketChat::ID);
        stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),sz);
        stream >> id >> p.message;

        // Then have the game handle it.
        if ( Game::Get()->CurrentState() == static_cast<unsigned int>(Game::Get()->GetGameOptionsStateID()) )
        {
	        UIElement * e = reinterpret_cast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOChatDisplayName ) ;
			static_cast< UIListbox * >( e )->AddItem( p.message ) ;
        }
        else
        {
	        Game::Get()->GetSession()->HandleChat(p.message);
        }
      }
      break;
	  case PacketKick::ID:
      {
        if ( !Game::Get()->GetSession()->IsHost() )
		{
          Game::Get()->WriteMessage("You have been kicked from the game, or the host has left the game.");
          Game::Get()->TransitionTo(Game::Get()->GetMainMenuStateID());
		}
	  }
	  break;
	  case PacketQuit::ID:
	  {
        if ( !Game::Get()->GetSession()->IsHost() )
		{
          Game::Get()->WriteMessage("I'm quitting now, thank you.");
          Game::Get()->TransitionTo(Game::Get()->GetMainMenuStateID());
		}
	  }
	  break;
      case PacketCueAdjust::ID:
      {
      PacketCueAdjust  p;
    
        // First, unmarshall the packet.
        ASSERT(*buffer == PacketCueAdjust::ID);
        stream.raw_set(reinterpret_cast< const nsl::byte_t* >(buffer),sz);
        stream >> id >> p.dx >> p.dy >> p.dz;
        
        Game::Get()->GetSession()->HandleCueAdjust(p.dx,p.dy,p.dz);
      }
      break;
      default:
      {
        // Bogus; ignore.
        ASSERT(false)("Bad packet sent to client.");
      }
    }
  }

  return (0);
}

/*  ________________________________________________________________________ */
void KeyHandler_MainMenu(int key)
/*! Keystroke handler for main menu / non-gameplay screens.
*/
{
  // Let the UI handle it.
  Game::Get()->GetScreen()->CheckKeyDown(key);
}

/*  ________________________________________________________________________ */
void LCHandler_MainMenu(int x,int y,int /*z*/)
/*! Left click handler for main menu / non-gameplay screens.
*/
{
  // Let the UI handle it.
  Game::Get()->GetScreen()->CheckLeftClick(x,y);
}

/*  ________________________________________________________________________ */
int UI_MMPlayClick(void)
/*! Invoked when the main menu "Play" button is clicked.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetGameSelectStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);

  return (0);
}

/*  ________________________________________________________________________ */
int UI_MMOptionsClick(void)
/*! Invoked when the main menu "Options" button is clicked.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetOptionsStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);

  return (0);
}

/*  ________________________________________________________________________ */
int UI_MMCreditsClick(void)
/*! Invoked when the main menu "Credits" button is clicked.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetCreditsStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);

  return (0);
}

/*  ________________________________________________________________________ */
int UI_MMQuitClick(void)
/*! Invoked when the main menu "Quit" button is clicked.
    
    @return
    Always zero.
*/
{
  ::PostQuitMessage(0);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GSBackClick(void)
/*! Invoked when the "back" button is clicked from game selection.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetMainMenuStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GSCreateGameClick(void)
/*! Invoked when the "Create Game" button is clicked from game selection.
    
    @return
    Always zero.
*/
{
  // Get options from the screen, as appropriate.
UIPanel     *p    = static_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GSPanelName));
std::string  name = static_cast< UIEditText* >(p->GetElement(kUI_GSPlayerName))->GetText();

  // Store options.
  Game::Get()->SetMyName(name);

  gTransferOnExit = Game::Get()->GetGameOptionsStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GSJoinGameClick(void)
/*! Invoked when the "Join Game" button is clicked from game selection.
    
    @return
    Always zero.
*/
{
  // Get the connection information for the game we are joining,
  // as well as local options.
UIPanel     *p  = reinterpret_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GSPanelName));
UIListbox   *lb = reinterpret_cast< UIListbox* >(p->GetElement(kUI_GSListName));
std::string  name = static_cast< UIEditText* >(p->GetElement(kUI_GSPlayerName))->GetText();
int          gi = lb->GetSelected();  
NetGameItr   g  = NetGamesBegin();

if ( gi >= 0 )
{
  // Store options.
  Game::Get()->SetMyName(name);

  for(int i = 0; i < gi; ++i)
    ++g;

  if ( g->second.numAvailPlayers >= 0 )
  {
	Game::Get()->SetMyServer(g->second.address);
	Game::Get()->SetMyPort(g->second.port);

	// Transfer.
	gTransferOnExit = Game::Get()->GetGameOptionsStateID();
	Game::Get()->GetScreen()->Exit(ExitFinished);
  }
}
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOBackClick(void)
/*! Invoked when the "back" button is clicked from game setup.
    
    @return
    Always zero.
*/
{
  // Communicate intentions to server
  if ( Game::Get()->GetSession()->IsHost() )
    NetServerSendQuit() ;
  else
    NetClientSendQuit() ;

  gTransferOnExit = Game::Get()->GetGameSelectStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);
  
  // If we were the host, we must unregister the game.
  if(Game::Get()->GetSession()->IsHost())
    NetGameUnregister();

  // Remove the session
  Game::Get()->DestroySession() ;

  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOStartClick(void)
/*! Invoked when the "start" button is clicked from game setup.
    
    @return
    Always zero.
*/
{
	  // 18-ball needs full two players for rules to work properly
	if ( gCurrentGameType == EIGHTEEN_BALL && Game::Get()->GetSession()->GetPlayer( 1 ) == 0 )
	{
        UIListbox * e = scast< UIListbox * >( scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOChatDisplayName ) ) ;
		e->AddItem( "!! Please set up a game with two players (human or AI) before starting a game of Eighteen-Ball." ) ;

		return 0 ;
	}

  // Only works for the host, obviously.
  if(Game::Get()->GetSession()->IsHost())
    NetServerSendStart();
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GONameLostFocus(void)
/*! Invoked when the game name edit box loses focus.
    
    @return
    Always zero.
*/
{
  // Set the game name.
UIPanel    *p  = reinterpret_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GOPanelName));
UIEditText *et = reinterpret_cast< UIEditText* >(p->GetElement(kUI_GOTitleName));

  Game::Get()->GetSession()->SetGameName(et->GetText());
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOChatLostFocus(void)
/*! Invoked when the chat entry entry box loses focus.
    
    @return
    Always zero.
*/
{
  // This will send the chat.
  UI_GOSendChatClick();

  // Clear the chat buffer.
UIPanel    *p  = reinterpret_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GOPanelName));
UIEditText *et = reinterpret_cast< UIEditText* >(p->GetElement(kUI_GOChatEntryName));

  et->SetText("");
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP0ButtonKickClick(void)
/*! Invoked when the button for the 1st player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  //UI_GOPButtonKickClick(0);

	// always host; ignore button presses here
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP1ButtonKickClick(void)
/*! Invoked when the button for the 2nd player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(1);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP2ButtonKickClick(void)
/*! Invoked when the button for the 3rd player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(2);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP3ButtonKickClick(void)
/*! Invoked when the button for the 4th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(3);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP4ButtonKickClick(void)
/*! Invoked when the button for the 5th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(4);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP5ButtonKickClick(void)
/*! Invoked when the button for the 6th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(5);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP6ButtonKickClick(void)
/*! Invoked when the button for the 7th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(6);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP7ButtonKickClick(void)
/*! Invoked when the button for the 8th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonKickClick(7);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOPButtonKickClick(int which)
/*! Invoked by handlers for specific button clicks.
    
    This function toggles the state of the player button and updates the game
    session accordingly.
    
    @return
    Always zero.
*/
{
UIPanel  *p = static_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GOPanelName));
UIButton *b = static_cast< UIButton* >(p->GetElement(kUI_GOPButtonName[which]));

  // If you are not the host, you cannot use this button. It should be
  // disabled, but just in case...
  if(!Game::Get()->GetSession()->IsHost())
    return (0);

  // If the clicked slot is empty and open, close the slot.
  if(Game::Get()->GetSession()->GetPlayer(which) == 0 && b->GetCaption() == kUI_GOSlotAvailBtnCap)
  {
    b->SetCaption(kUI_GOSlotCloseBtnCap);
    
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPKickButtonName[which]));
    b->SetCaption(kUI_GOOpenBtnCap);
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPAddAIButtonName[which]));
    b->Enable(false);
  }
  // If the clicked slot is empty and closed, open the slot.
  else if(Game::Get()->GetSession()->GetPlayer(which) == 0 && b->GetCaption() == kUI_GOSlotCloseBtnCap)
  {
	b->SetCaption(kUI_GOSlotAvailBtnCap);

    b = static_cast< UIButton* >(p->GetElement(kUI_GOPKickButtonName[which]));
    b->SetCaption(kUI_GOCloseBtnCap);
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPAddAIButtonName[which]));
    b->Enable(true);
  }
  // If the clicked slot contains a player, kick it.
  else if(Game::Get()->GetSession()->GetPlayer(which) != 0)
  {
      // Kick the player.
    NetServerSendKick(which);

	  // set the interface to an open slot
	b->SetCaption(kUI_GOSlotAvailBtnCap);
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPKickButtonName[which]));
    b->SetCaption(kUI_GOCloseBtnCap);
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPAddAIButtonName[which]));
    b->Enable(true);
  }
  
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP0ButtonAddAIClick(void)
/*! Invoked when the button for the 1st player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  //UI_GOPButtonAddAIClick(0);

	// always host; ignore clicks here
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP1ButtonAddAIClick(void)
/*! Invoked when the button for the 2nd player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(1);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP2ButtonAddAIClick(void)
/*! Invoked when the button for the 3rd player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(2);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP3ButtonAddAIClick(void)
/*! Invoked when the button for the 4th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(3);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP4ButtonAddAIClick(void)
/*! Invoked when the button for the 5th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(4);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP5ButtonAddAIClick(void)
/*! Invoked when the button for the 6th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(5);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP6ButtonAddAIClick(void)
/*! Invoked when the button for the 7th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(6);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOP7ButtonAddAIClick(void)
/*! Invoked when the button for the 8th player is clicked.
    
    This function wraps a more general version that actually deals with
    the clicks.
    
    @return
    Always zero.
*/
{
  UI_GOPButtonAddAIClick(7);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOPButtonAddAIClick(int which)
/*! Invoked by handlers for specific button clicks.
    
    This function toggles the state of the player button and updates the game
    session accordingly.
    
    @return
    Always zero.
*/
{
UIPanel  *p = static_cast< UIPanel* >(Game::Get()->GetScreen()->GetElement(kUI_GOPanelName));
UIButton *b = static_cast< UIButton* >(p->GetElement(kUI_GOPButtonName[which]));

  // If the clicked slot is available, put an AI player in it.
  // Otherwise do nothing. (available == no player and not closed)
  if(Game::Get()->GetSession()->GetPlayer(which) == 0 &&
	 static_cast< UIButton* >(p->GetElement(kUI_GOPAddAIButtonName[which]))->IsEnabled() )
  {
    b->SetCaption(kUI_GOSlotAIBtnCap);
    Game::Get()->GetSession()->SetPlayer(which,Game::Get()->GetAIPlayer());
    
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPKickButtonName[which]));
    b->SetCaption(kUI_GOKickBtnCap);
    b = static_cast< UIButton* >(p->GetElement(kUI_GOPAddAIButtonName[which]));
    b->Enable(false);
  }
  
  ///////////////////////////////
  NetServerAddPlayer(which);
  ///////////////////////////////
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GOSendChatClick( void )
/*!
	\brief		Sends a pre-game chat message

	\retval		int		always zero
 */
{
	UIElement * e = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) )->GetElement( kUI_GOChatEntryName ) ;
	NetClientSendChat( static_cast< UIEditText * >( e )->GetText() ) ;
	Game::Get()->GetScreen()->Focus( e ) ;

	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Disables enough of the player list entries that only the max
	            supported players for a game can join (ie 2 for 8-ball, 8 for
				the rest)

	\param      p_gameType      The game type to use in determining which
	                            slots to open or close
 */
static void TweakPlayerList( eGameType p_gameType )
{
	ASSERT( p_gameType < GAME_TYPE_COUNT ) ;
	ASSERT( GameMaxPlayers[ p_gameType ] >= 2 ) ;
	ASSERT( GameMaxPlayers[ p_gameType ] <= kPlayersMax ) ;

	for ( unsigned int i = 1 ; i < kPlayersMax ; ++i )
	{
		Player *    player     = Game::Get()->GetSession()->GetPlayer( i ) ;
		UIPanel *   panelGO    = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) ) ;
		UIElement * editName   = scast< UIElement * >( panelGO->GetElement( kUI_GOPButtonName[ i ] ) ) ;
		UIElement * buttonKick = scast< UIElement * >( panelGO->GetElement( kUI_GOPKickButtonName[ i ] ) ) ;
		UIElement * buttonAI   = scast< UIElement * >( panelGO->GetElement( kUI_GOPAddAIButtonName[ i ] ) ) ;

		if ( i < GameMaxPlayers[ p_gameType ] )
		{
			editName->Enable( true ) ;

			if ( Game::Get()->GetSession()->IsHost() )
			{
				if ( player == 0 && editName->GetCaption() == kUI_GOSlotAvailBtnCap )
				{
					  // Slot is available
					buttonKick->Enable( true ) ;
					buttonAI->Enable( true ) ;
				}
				else
				{
					  // Slot is closed, a human player, or an AI player
					buttonKick->Enable( true ) ;
					buttonAI->Enable( false ) ;
				}
			}
			else
			{
				  // Slot modification is unavailable to clients
				buttonKick->Enable( false ) ;
				buttonAI->Enable( false ) ;
			}
		}
		else
		{
			if ( Game::Get()->GetSession()->IsHost() && player != 0 )
				UI_GOPButtonKickClick( i ) ;

			editName->Enable( false ) ;
			buttonKick->Enable( false ) ;
			buttonAI->Enable( false ) ;
		}
	}
}

/*  ________________________________________________________________________ */
/*!
	\brief		Sets the game type to the first one

	\param      p_type  Selected game type
 */
void UI_GOGameTypeXClick( eGameType p_type )
{
	UIPanel *   p   = rcast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) ) ;
	UIElement * gt[ GAME_TYPE_COUNT ] ;  // _g_ame _t_ype on/off images
	UIPanel *   dp[ GAME_TYPE_COUNT ] ;  // _d_escription _p_anels
	for ( int i = 0 ; i < GAME_TYPE_COUNT ; ++i )
	{
		gt[ i ] = p->GetElement( kUI_GOGameTypeImgName[ i ] ) ;
		dp[ i ] = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ i ] ) ) ;
	}

	for ( int i = 0 ; i < GAME_TYPE_COUNT ; ++i )
	{
		if ( i == scast< int >( p_type ) )
		{
			  // Turn on this radio button
			gt[ i ]->RectTexture( kUI_RadioBtnOnStr ) ;

			  // Display appropriate description
			dp[ i ]->Die( false ) ;
			dp[ i ]->Reset() ;
		}
		else
		{
			  // Turn off all the rest
            gt[ i ]->RectTexture( kUI_RadioBtnOffStr ) ;

			  // Hide the rest of the descriptions
			dp[ i ]->Exit() ;
			dp[ i ]->Die( true ) ;
		}
	}

	gCurrentGameType = p_type ;
	TweakPlayerList( scast< eGameType >( gCurrentGameType ) ) ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Sets the game type to the first one

	\retval		int		always zero
 */
int UI_GOGameType1Click( void )
{
	UI_GOGameTypeXClick( EIGHTEEN_BALL ) ;
	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Sets the game type to the first one

	\retval		int		always zero
 */
int UI_GOGameType2Click( void )
{
	UI_GOGameTypeXClick( NINETEEN_BALL ) ;
	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Sets the game type to the first one

	\retval		int		always zero
 */
/*
int UI_GOGameType3Click( void )
{
	UIPanel *   p   = rcast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) ) ;
	UIElement * gt0 = p->GetElement( kUI_GOGameTypeImgName[ 0 ] ) ;
	UIElement * gt1 = p->GetElement( kUI_GOGameTypeImgName[ 1 ] ) ;
	UIElement * gt2 = p->GetElement( kUI_GOGameTypeImgName[ 2 ] ) ;
	UIElement * gt3 = p->GetElement( kUI_GOGameTypeImgName[ 3 ] ) ;
	UIPanel *   dp0 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 0 ] ) ) ;
	UIPanel *   dp1 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 1 ] ) ) ;
	UIPanel *   dp2 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 2 ] ) ) ;
	UIPanel *   dp3 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 3 ] ) ) ;

	  // Turn on this radio button
	gt2->RectTexture( kUI_RadioBtnOnStr ) ;

	  // Turn off all the rest
	gt0->RectTexture( kUI_RadioBtnOffStr ) ;
	gt1->RectTexture( kUI_RadioBtnOffStr ) ;
	gt3->RectTexture( kUI_RadioBtnOffStr ) ;

	  // Display appropriate description
	dp2->Die( false ) ;
	dp2->Reset() ;

	  // Hide the rest of the descriptions
	dp0->Exit() ;
	dp0->Die( true ) ;
	dp1->Exit() ;
	dp1->Die( true ) ;
	dp3->Exit() ;
	dp3->Die( true ) ;

	gCurrentGameType = 2 ;
	TweakPlayerList( scast< eGameType >( gCurrentGameType ) ) ;

	return 0 ;
}
*/

/*  ________________________________________________________________________ */
/*!
	\brief		Sets the game type to the first one

	\retval		int		always zero
 */
/*
int UI_GOGameType4Click( void )
{
	UIPanel *   p   = rcast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOPanelName ) ) ;
	UIElement * gt0 = p->GetElement( kUI_GOGameTypeImgName[ 0 ] ) ;
	UIElement * gt1 = p->GetElement( kUI_GOGameTypeImgName[ 1 ] ) ;
	UIElement * gt2 = p->GetElement( kUI_GOGameTypeImgName[ 2 ] ) ;
	UIElement * gt3 = p->GetElement( kUI_GOGameTypeImgName[ 3 ] ) ;
	UIPanel *   dp0 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 0 ] ) ) ;
	UIPanel *   dp1 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 1 ] ) ) ;
	UIPanel *   dp2 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 2 ] ) ) ;
	UIPanel *   dp3 = scast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GOGameTypePnlName[ 3 ] ) ) ;

	  // Turn on this radio button
	gt3->RectTexture( kUI_RadioBtnOnStr ) ;

	  // Turn off all the rest
	gt0->RectTexture( kUI_RadioBtnOffStr ) ;
	gt1->RectTexture( kUI_RadioBtnOffStr ) ;
	gt2->RectTexture( kUI_RadioBtnOffStr ) ;

	  // Display appropriate description
	dp3->Die( false ) ;
	dp3->Reset() ;

	  // Hide the rest of the descriptions
	dp0->Exit() ;
	dp0->Die( true ) ;
	dp1->Exit() ;
	dp1->Die( true ) ;
	dp2->Exit() ;
	dp2->Die( true ) ;

	gCurrentGameType = 3 ;
	TweakPlayerList( scast< eGameType >( gCurrentGameType ) ) ;

	return 0 ;
}
*/

/*  ________________________________________________________________________ */
int UI_GPEnterGameplay(void)
/*! Invoked when its time to transition to the game UI elements.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetGameplayStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GPResumeClick(void)
/*! Invoked when the "resume game" button is clicked from the gameplay menu.
*/
{
  // Simply hide the menu.
  Game::Get()->GetSession()->HideMenu();
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GPMenuResumeClick(void)
/*! Invoked when the "resume game" button is clicked from the gameplay menu.
*/
{
  // Simply hide the menu.
  Game::Get()->GetSession()->HideMenu();
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GPMenuResignClick(void)
/*! Invoked when the "resign" button is clicked from the gameplay menu.
*/
{
  // Communicate intentions to server
  if ( Game::Get()->GetSession()->IsHost() )
    NetServerSendQuit() ;
  else
    NetClientSendQuit() ;

  // Return to main menu.
  Game::Get()->WriteMessage("I'm resigning now, thank you.");
  Game::Get()->TransitionTo(Game::Get()->GetMainMenuStateID());
  return (0);
}

/*  ________________________________________________________________________ */
int UI_GPMenuQuitClick(void)
/*! Invoked when the "quit" button is clicked from the gameplay menu.
*/
{
  // Abandon ship.
  //Game::Get()->WriteMessage("I'm quitting now, thank you.");
  ::PostQuitMessage(0);
  return (0);
}

/*  ________________________________________________________________________ */
/*!
	\brief		Displays or hides scoreboard panel

	\retval		int		always 0
 */
int UI_GPSBToggleClick( void )
{
	UIPanel  * p = rcast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GPScoreboardPanelName ) ) ;
	UIButton * b = rcast< UIButton * >( Game::Get()->GetScreen()->GetElement( kUI_GPScoreboardToggleName ) ) ;
	if ( b->GetCaption() == kUI_GPScoreboardBtnCap )
	{
          // Refresh scoreboard data
		for ( int i = 0 ; i < kPlayersMax ; ++i )
		{
			Player * player = Game::Get()->GetSession()->GetPlayer( i ) ;
			Rules *  rules  = Game::Get()->GetSession()->GetRules() ;

		    if ( player != 0 )
			{
				UIElement *e ;

				  // Update player names
				e = p->GetElement( kUI_GPSBPlayerTextName[ i ] ) ;
				e->SetCaption( player->GetName() ) ;

				  // Update scores
				e = p->GetElement( kUI_GPSBScoreTextName[ i ] ) ;
				std::ostringstream score ;
				score << rules->GetScore( i ) ;
				scast< UIEditText * >( e )->SetText( score.str() ) ;
			}
			else
			{
				UIElement *e ;

				  // Update player names
				e = p->GetElement( kUI_GPSBPlayerTextName[ i ] ) ;
				e->SetCaption( "(empty)" ) ;

				  // Update scores
				e = p->GetElement( kUI_GPSBScoreTextName[ i ] ) ;
				std::ostringstream score ;
				score << 0 ;
				scast< UIEditText * >( e )->SetText( score.str() ) ;
			}
		}

		p->Die( false ) ;
		p->Reset() ;
		b->SetCaption( kUI_GPHideVertBtnCap ) ;
	}
	else
	{
		p->Exit() ;
		p->Die( true ) ;
		b->SetCaption( kUI_GPScoreboardBtnCap ) ;
	}

	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Displays or hides jukebox panel

	\retval		int		always 0
 */
int UI_GPJBToggleClick( void )
{
	UIPanel  * p = rcast< UIPanel * >( Game::Get()->GetScreen()->GetElement( kUI_GPJukeboxPanelName ) ) ;
	UIButton * b = rcast< UIButton * >( Game::Get()->GetScreen()->GetElement( kUI_GPJukeboxToggleName ) ) ;
	if ( b->GetCaption() == kUI_GPJukeboxBtnCap )
	{
		p->Die( false ) ;
		p->Reset() ;
		b->SetCaption( kUI_GPHideVertBtnCap ) ;
	}
	else
	{
		p->Exit() ;
		p->Die( true ) ;
		b->SetCaption( kUI_GPJukeboxBtnCap ) ;
	}

	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Plays selected song in the jukebox playlist

	\retval		int		always 0
 */
int UI_GPJBPlayClick(void)
{
	UIListbox * playlist = scast< UIListbox * >(
		scast< UIPanel * >( Game::Get()->GetScreen( "Gameplay" )->GetElement(
			kUI_GPJukeboxPanelName ) )->GetElement( kUI_GPJBPlaylistName ) ) ;

	int                         selection = playlist->GetSelected() ;
	std::vector< Game::JBInfo > songs     = Game::Get()->GetJBSongs() ;

	if ( selection >= 0 )
	{
		ASSERT( selection < scast< signed int >( songs.size() ) ) ;
		if ( songs[ selection ].m_songID < 0 )
		{
			  // Load in the file and register it with the sound manager
			songs[ selection ].m_songID = Game::Get()->GetSound()->Load2DObject(
				const_cast< char * >( ( songs[ selection ].m_songFilename).c_str() ) ) ;
		}
		Game::Get()->GetSound()->StopObject( Game::Get()->GetCurrentJBSong() ) ;
		Game::Get()->GetSound()->PlayObject( songs[ selection ].m_songID ) ;
		Game::Get()->SetCurrentJBIndex( selection ) ;
		Game::Get()->SetCurrentJBSong( songs[ selection ].m_songID ) ;

		  // Update "Currently Playing" line
		UIEditText * currentlyPlaying = scast< UIEditText * >(
			scast< UIPanel * >( Game::Get()->GetScreen( "Gameplay" )->GetElement(
				kUI_GPJukeboxPanelName ) )->GetElement( kUI_GPJBCurSongDetailsName ) ) ;
		currentlyPlaying->SetText( songs[ selection ].m_songFilename ) ;
	}

	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Stops currently playing song

	\retval		int		always 0
 */
int UI_GPJBStopClick(void)
{
	Game::Get()->GetSound()->StopObject( Game::Get()->GetCurrentJBSong() ) ;
	Game::Get()->SetCurrentJBIndex( -1 ) ;
	Game::Get()->SetCurrentJBSong( -1 ) ;

	  // Update "Currently Playing" line
	UIEditText * currentlyPlaying = scast< UIEditText * >(
		scast< UIPanel * >( Game::Get()->GetScreen( "Gameplay" )->GetElement(
			kUI_GPJukeboxPanelName ) )->GetElement( kUI_GPJBCurSongDetailsName ) ) ;
	currentlyPlaying->SetText( "" ) ;
	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Plays previous song (relative to currently playing song) in the
	            jukebox playlist

	\retval		int		always 0
 */
int UI_GPJBPrevClick(void)
{
	UIListbox * playlist = scast< UIListbox * >(
		scast< UIPanel * >( Game::Get()->GetScreen( "Gameplay" )->GetElement(
			kUI_GPJukeboxPanelName ) )->GetElement( kUI_GPJBPlaylistName ) ) ;

	if ( Game::Get()->GetCurrentJBIndex() <= 0 )
		playlist->SetSelected( Game::Get()->GetJBSongs().size() - 1 ) ;
	else
		playlist->SetSelected( Game::Get()->GetCurrentJBIndex() - 1 ) ;
	UI_GPJBPlayClick() ;

	return 0 ;
}

/*  ________________________________________________________________________ */
/*!
	\brief		Plays next song (relative to currently playing song) in the
	            jukebox playlist

	\retval		int		always 0
 */
int UI_GPJBNextClick(void)
{
	UIListbox * playlist = scast< UIListbox * >(
		scast< UIPanel * >( Game::Get()->GetScreen( "Gameplay" )->GetElement(
			kUI_GPJukeboxPanelName ) )->GetElement( kUI_GPJBPlaylistName ) ) ;

	if ( Game::Get()->GetCurrentJBIndex() >= scast<int>( Game::Get()->GetJBSongs().size() - 1 ) )
        playlist->SetSelected( 0 ) ;
	else
        playlist->SetSelected( Game::Get()->GetCurrentJBIndex() + 1 ) ;
	UI_GPJBPlayClick() ;

	return 0 ;
}

/*  ________________________________________________________________________ */
int UI_OptsSaveClick(void)
/*! Invoked when the "Apply" button is clicked from the options screen.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetMainMenuStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_OptsBackClick(void)
/*! Invoked when the "Back" button is clicked from the options screen.
    
    @return
    Always zero.
*/
{
  gTransferOnExit = Game::Get()->GetMainMenuStateID();
  Game::Get()->GetScreen()->Exit(ExitFinished);
  return (0);
}

/*  ________________________________________________________________________ */
int UI_DontClick(void)
/*! 
*/
{
  UpdateTrackServer();
  return (0);
}


void Physics_OnPlaneContactCB(Collision::Contact* c,Physics::RigidBody *p, Physics::RigidBody * /*s*/)
{
	//c->mBody1 = s;
	Geometry::Vector3D  pn = reinterpret_cast< Physics::Plane* >(p)->mPlane.normal();
	
	// Is this the right plane?
	if(pn[0] - 1.0 < 0.001)
	  Game::Get()->GetPlayfield()->mPlanes[4]->Disturb(-c->mPosition[2],
	                                                    c->mPosition[1]);

	static bool first = true;
	static int i;
	if (first == true)
	{
		first = false;
		i = Game::Get()->GetSound()->Load2DObject("data/sound/WaterDrip01.wav");
	}
	Game::Get()->GetSound()->PlayObject(i);

	// get velocity
	int ball_id;
	ball_id = (c->mBody1->mCollideGeom->Kind() == Physics::kC_Sphere) ? c->mID1 : c->mID2;
	Geometry::Vector3D vel = Game::Get()->GetPhysics()->RigidBodyVector3D(ball_id, Physics::Engine::eRigidBodyVector::propVeloctity);
	double v = vel.length()/10.0;

	// make shockwave
	//if( v > 1 )
	//{
		CreatePShockwave(	Game::Get()->GetParticles(),
							Geometry::Point3D(0.0,0.0,0.0) + c->mPosition,
                            c->mNormal, v * (c->mNormal.normal() * vel.normal()));
	//}
}

void Physics_OnPocketContactCB(Collision::Contact* c, Physics::RigidBody * /*p*/, Physics::RigidBody * /*s*/)
{
	unsigned int pocket = 0, ball = 0;
    bool ball_pocketed = false;
    Physics::RigidBody* pPocket = 0, *pBall = 0;

    if(c->mBody1->mCollideGeom->Kind() == Physics::kC_BoundedPlane)
    {
        pocket = c->mID1;
        ball = c->mID2;
        pPocket = c->mBody1;
        pBall = c->mBody2;
    }
    else
    {
        pocket = c->mID2;
        ball = c->mID1;
        pPocket = c->mBody2;
        pBall = c->mBody1;
    }
    
    if(std::find(Game::Get()->GetPlayfield()->mPocketedBalls.begin(), Game::Get()->GetPlayfield()->mPocketedBalls.end(), ball) != Game::Get()->GetPlayfield()->mPocketedBalls.end())
        return;

	for(size_t i = 0; i < Game::Get()->GetPlayfield()->mPockets.size(); ++i)
	{
        if(Game::Get()->GetPlayfield()->mPockets[i]->QueryPlane(pocket))
        {
            Game::Get()->GetPlayfield()->mPocketedBalls.push_back(ball);
            GetBallByNumber(GetBallNumber(ball))->Pocketed(true);
            ball_pocketed = true;
        }
		
	}
	static bool first = true;
	static int j;
	if (first == true)
	{
		first = false;
		j = Game::Get()->GetSound()->Load2DObject("data/sound/ArcadeAlarm01.wav");
	}
    if(ball_pocketed)
    {
	    Game::Get()->GetSound()->PlayObject(j);
	Geometry::Point3D org(c->mPosition[0], c->mPosition[1], c->mPosition[2]);
    CreatePExplosion( Game::Get()->GetParticles(), org, 50 );
//	CreatePMarker( Game::Get()->GetParticles(), org );
	++Game::Get()->ti.get( "Pocket Intersections" );
    }
}


void Physics_OnSphereContactCB(Collision::Contact* c,Physics::RigidBody * /*s1*/, Physics::RigidBody * /*s2*/)
{
	static bool first = true;
	static int i;
	if (first == true)
	{
		first = false;
        i = Game::Get()->GetSound()->Load2DObject("data/sound/balls.wav");
        
	}
    /*float scaleVol = 1.f - ((c->mBody1->mStateT1.mVelocity.normal() * c->mBody2->mStateT1.mVelocity.normal()) + 1.f)/2.f;
    float mag = abs(c->mBody1->mStateT1.mVelocity.length() - c->mBody2->mStateT1.mVelocity.length())/(2.f*Game::Get()->GetPhysics()->GetMaxLinearVelocity());
    char vol = scast<char>(mag * 255.f * scaleVol);
    Game::Get()->GetSound()->SetVolume(i, 0);*/
    Game::Get()->GetSound()->PlayObject(i);

	// get velocity
	Geometry::Vector3D vel1 = Game::Get()->GetPhysics()->RigidBodyVector3D(c->mID1, Physics::Engine::eRigidBodyVector::propVeloctity);
	Geometry::Vector3D vel2 = Game::Get()->GetPhysics()->RigidBodyVector3D(c->mID2, Physics::Engine::eRigidBodyVector::propVeloctity);
	double v = (vel1 + vel2).length()/40.0;

	// make shockwave
	if( v > 1 )
	{
		/*CreatePShockwave(	Game::Get()->GetParticles(),
							Geometry::Point3D(0.0,0.0,0.0) + c->mPosition,
							c->mNormal, v );
        */
    }                        
}

/*  ________________________________________________________________________ */
void Physics_OnStaticCB(Collision::Contact* /*c*/, Physics::RigidBody * /*p*/, Physics::RigidBody * /*s*/)
/*! Invoked when the physics simulation transitions from moving to static.

    @param c
    @param p
    @param s
*/
{
std::vector< D3DXVECTOR3 >  balls;
std::vector< char >         pflags;
  
  // Test the rules.
  if(Game::Get()->GetSession()->GetRules())
  {
    if(Game::Get()->GetSession()->GetRules()->Test()) // is this a legal shot?
    {
      Game::Get()->GetSession()->GetRules()->HandleScratch(Game::Get()->GetSession()->GetRules()->GetLastScratch());
    }
    else
    {
      Game::Get()->GetSession()->GetRules()->HandleScratch(Game::Get()->GetSession()->GetRules()->GetLastScratch());
      Game::Get()->GetSession()->AdvanceTurn();	// if the shot was not successful, advance the turn
    }
  }
  
  // Sync the balls.
  if(Game::Get()->GetSession() != 0 && Game::Get()->GetSession()->IsHost())
  {
    for(unsigned int i = 0 ; i < Game::Get()->GetPlayfield()->mBalls.size(); ++i)
    {
    Geometry::Vector3D gv = Game::Get()->GetPhysics()->RigidBodyVector3D(Game::Get()->GetPlayfield()->mBalls[i]->ID(),Physics::Engine::eRigidBodyVector::propPosition);
   // char fucker=Game::Get()->GetPlayfield()->mBalls[i]->Pocketed();
      balls.push_back(D3DXVECTOR3(gv[0],gv[1],gv[2]));
      pflags.push_back(Game::Get()->GetPlayfield()->mBalls[i]->Pocketed());
    }
    //NetServerSendSync(balls,pflags);
  }

int t =Game::Get()->GetSession()->CurrentTurn();
Player *pa = Game::Get()->GetSession()->GetPlayer(t);

  if(pa != 0 && pa->IsAI() && Game::Get()->GetSession()->IsHost())
  {
    // Take the AI player's turn.
    // Spot the cue ball if the last player scratched.
    if(Game::Get()->needToSpot)
    {
      Game::Get()->GetPhysics()->RigidBodyVector3D(GetBallByNumber(0)->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0,0,-25));
      Game::Get()->needToSpot = false;
    } 
    
  Shot sh = Game::Get()->GetAIPlayer()->SelectShot();
  D3DXVECTOR3  v(sh.v[0],sh.v[1],sh.v[2]);
  
    NetClientSendTurn(v,60);
  }
  
}
