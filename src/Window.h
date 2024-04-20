/*! ========================================================================
    
      @file   Window.h
      @author jmc,jmp
      @brief  Interface to window class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _WINDOW_H_
#define _WINDOW_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <windows.h>

#include <string>
#include <map>

#include "asserter.h"
#include "enforcer.h"

#include "GraphicsRenderer.h"


/*                                                                  includes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Window
/*! Wrapper class for a DirectX-enabled window.
*/
{
  public:
    // typedefs
    // typedefs
    typedef LRESULT (CALLBACK *Callback)(Window*,UINT,WPARAM,LPARAM);
    
    // ct and dt
    Window(void);
    ~Window(void);
    
    // components
    Graphics::Renderer* GetRenderer(void) { ASSERT(0 != mRenderer); return (mRenderer); }
    HWND                GetHandle(void)   { ASSERT(0 != mWind); return (mWind); }
    
    // accessors
    int          Width(void) const  { return (mW); } 
    int          Height(void) const { return (mH); }
    std::string  Title(void) const  { return (mTitle); }
    
    // manipulators
    void Title(const std::string &t) { mTitle = t; ::SetWindowText(mWind,mTitle.c_str()); }
    
    // callbacks
    void InstallCallback(UINT msg,Callback cb);


	void Begin2DRendering();
	void End2DRendering();
	
  private:
    // typdefs
    typedef std::map< UINT,Callback >  CallbackMap;  //!< Maps message types to callbacks.
    
    // windows message handler
    static LRESULT CALLBACK nWinProc(HWND wind,UINT msg,WPARAM wp,LPARAM lp);
    
    // window class registration
    static void nRegisterWindowClass(void);
    static void nUnRegisterWindowClass(void);
    
    // data members
    HWND                mWind;      //!< Window handle.
    Graphics::Renderer *mRenderer;  //!< Graphics engine.

    std::string  mTitle;  //!< Title of the window.
    int          mW;      //!< Width of the window's render area.
    int          mH;      //!< Height of the window's render area.

    CallbackMap  mCallbacks;  //!< User-supplied callbacks for Windows messages.
    
    static HANDLE         smRegEvt;     //!< Window class register event.
    static HANDLE         smUnRegEvt;   //!< Window class unregister event.
    static unsigned long  smWindCount;  //!< Number of windows, for registration tracking.
  };

#endif  /* _WINDOW_H_ */
