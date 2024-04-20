/*! ========================================================================
    
      @file    main.cpp
      @author  jmp
      @brief   Corner Hooked main file.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"

#include "Profiler.h"
#include "Log.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

Profiler      p;
LogSingleton  ls;


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
int WINAPI WinExceptInfo(LPEXCEPTION_POINTERS ptrs)
/*! Exception record processor.
    
    @param ptrs  Information describing the exception.
    
    @return
    Always EXCEPTION_EXECUTE_HANDLER.
*/
{
	dbg::stacktrace(*(ptrs->ContextRecord));
	return (EXCEPTION_EXECUTE_HANDLER);
}

/*  ________________________________________________________________________ */
int WINAPI WinMainHandled(HINSTANCE /*thisInst*/,HINSTANCE /*prevInst*/,LPSTR /*cmdLine*/,int /*cmdShow*/)
/*! SEH-wrapped application entry point.

    @param thisInst  Active application instance.
    @param prevInst  Previous application instance.
    @param cmdLine   Command line argument buffer.
    @param cmdShow   Command for window appearance.

    @return
    A result code.
*/
{
ProfileFn;

  try
  {
  bool   done = false;  
  MSG    msg;           
  Game   game;
  
    game.Init(); 
    while(!done)
    {
      if(::PeekMessage(&msg,0,0,0,PM_REMOVE))
      {
        if(msg.message == WM_QUIT)
        {
		  if ( game.GetSession() )
		  {
			// Communicate intentions to server
			if ( Game::Get()->GetSession()->IsHost() )
				NetServerSendQuit() ;
			else
				NetClientSendQuit() ;
		  }

          done = true;
          continue;
        }
        else
        {
          ::TranslateMessage(&msg);
          ::DispatchMessage(&msg);
        }
      }
      
      // Deal with one playloop cycle.
      Game::Get()->Playloop();
    }
  }
  catch(std::exception &e)
  {
    ::MessageBox(0,e.what(),"Error",MB_OK);
    throw;
  }
  catch(...)
  {
    // Re-throw and let the SEH block handle it (it will dump the stack for us).
    throw;
  }
  
  return 0;
}

/*  ________________________________________________________________________ */
int WINAPI WinMain(HINSTANCE thisInst,HINSTANCE prevInst,LPSTR cmdLine,int cmdShow)
/*! Application entry point.

    @param thisInst  Active application instance.
    @param prevInst  Previous application instance.
    @param cmdLine   Command line argument buffer.
    @param cmdShow   Command for window appearance.

    @return
    A result code.
*/
{

int  result = 0;

  dbghelp::load();
  
  __try
  {
    result = WinMainHandled(thisInst,prevInst,cmdLine,cmdShow);
  }
  __except(WinExceptInfo(GetExceptionInformation()))
  {
    result = -1;
  }
  
  dbghelp::free(); 
  return (result);
}