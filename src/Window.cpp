/*! ========================================================================
    
      @file   Window.cpp
      @author jmc,jmp
      @brief  Implementation of window class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Window.h"
#include "log.h"

/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const char         kWndClassName[] = "CHWndClass";     //!< Name of the window class.
  const std::string  kWndTitle       = "Corner Hooked";  //!< Title of the window.
}


/*                                                                 variables
---------------------------------------------------------------------------- */

unsigned long  Window::smWindCount = 0;
HANDLE         Window::smRegEvt    = ::CreateEvent(0,false,true,0);
HANDLE         Window::smUnRegEvt  = ::CreateEvent(0,false,true,0);


/*                                                                  includes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Window::Window(void)
/*! Default constructor.
*/
: mTitle(kWndTitle)
{
  // First window needs to do window class registration.
  // Prevent the class from registering twice by waiting for
  // the "can register" signal.
  ::WaitForSingleObject(smRegEvt,INFINITE);
  if(smWindCount++ == 0)
    nRegisterWindowClass();
  ::SetEvent(smRegEvt);
  

	mW = ::GetSystemMetrics(SM_CXSCREEN);
	mH = ::GetSystemMetrics(SM_CYSCREEN);
	if(mW < 1024 || mH < 768)
	{
	  mW = 1024;
	  mH = 768;
	}

#if defined(_DEBUG)
int windFlags = WS_OVERLAPPEDWINDOW;
#else
int windFlags = WS_POPUP;
#endif

	// Create a window.
	mWind = CreateWindowEx(0,kWndClassName,mTitle.c_str(),
	                       windFlags,
		                     CW_USEDEFAULT, CW_USEDEFAULT,mW,mH,
		                     0,0,::GetModuleHandle(0),this);

RECT foo;
  ::GetClientRect(mWind,&foo);

#if defined(_DEBUG)  
	mW = foo.right;
	mH = foo.bottom;
#endif

	ShowWindow(mWind,SW_SHOW);
	UpdateWindow(mWind);

  // Construct the graphics engine.
	mRenderer = new Graphics::Renderer(mWind,D3DFMT_X8R8G8B8,D3DFMT_D16);
#if defined(_DEBUG)  
	mRenderer->SetDisplayMode(mW,mH,60,false);
#else
  mRenderer->SetDisplayMode(mW,mH,0,true);
#endif
}

/*  ________________________________________________________________________ */
Window::~Window()
/*! Destructor.
*/
{
  // Last window destroys the window class on its way out.
  // Same threading caveats as when we registered the class.
  ::WaitForSingleObject(smUnRegEvt,INFINITE);
  if(--smWindCount == 0)
    nUnRegisterWindowClass();
  ::SetEvent(smUnRegEvt);
  
  // Clean up the window.
  if(::IsWindow(mWind))
	  ::DestroyWindow(mWind);
	SAFE_DELETE(mRenderer);
}

/*  ________________________________________________________________________ */
void Window::InstallCallback(UINT msg,Window::Callback cb)
/*! Install a callback for a Windows message.

    @param msg  The message code.
    @param cb   The callback to be invoked when the above message is handled.
*/
{
  mCallbacks[msg] = cb;
}

/*  ________________________________________________________________________ */
LRESULT CALLBACK Window::nWinProc(HWND wind,UINT msg,WPARAM wp,LPARAM lp)
/*! Message handler.

    @param wind  The window handle.
    @param msg   The message code.
    @param wp    Message payload field one.
    @param lp    Message payload field two.
    
    @return
    A result code, the value of which depends on the message handled.
*/
{
  // Store the window pointer on creation.
  if(msg == WM_CREATE)
  {
    ::SetWindowLongPtr(wind,0,reinterpret_cast< LONG_PTR >((*reinterpret_cast< LPCREATESTRUCT >(lp)).lpCreateParams));
    return (0);
  }
  else if(msg == WM_SETCURSOR)
  {
    ::SetCursor(::LoadCursor(0,IDC_ARROW));
    return (true);
  }
 
  // Recover the window pointer.
Window *ptr = reinterpret_cast< Window* >(::GetWindowLongPtr(wind,0));
    
  // On destruction, disable the renderer so it doesn't try to draw
  // to a dead window.
  if(msg == WM_DESTROY)
  {
    ptr->mRenderer->CanRender(false);
    LogS->Dump();
  }
  
  if(0 != ptr)
  {
  CallbackMap::iterator  cb = ptr->mCallbacks.find(msg);
  
    // If a user-installed callback exists for the message, call it, otherwise
    // let the defaults handle it.
    if(cb != ptr->mCallbacks.end() && cb->second != 0)
      cb->second(ptr,msg,wp,lp);
  }

  return (::DefWindowProc(wind,msg,wp,lp));
}

/*  ________________________________________________________________________ */
void Window::nRegisterWindowClass(void)
/*! Register the window class.
*/
{
WNDCLASSEX  wc;

  wc.cbSize	       = sizeof(WNDCLASSEX);
  wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc   = nWinProc;
  wc.cbClsExtra	   = 0;
  wc.cbWndExtra	   = sizeof(Window*);
  wc.hInstance     = ::GetModuleHandle(0);
  wc.hIcon         = 0;
  wc.hCursor       = 0;
  wc.hbrBackground = static_cast< HBRUSH >(::GetStockObject(BLACK_BRUSH));
  wc.lpszMenuName	 = 0;
  wc.lpszClassName = kWndClassName;
  wc.hIconSm       = ::LoadIcon(0,IDI_APPLICATION);
  ::RegisterClassEx(&wc);
}

/*  ________________________________________________________________________ */
void Window::nUnRegisterWindowClass(void)
/*! Unregister the window class.
*/
{
  ::UnregisterClass(kWndClassName,::GetModuleHandle(0));
}
