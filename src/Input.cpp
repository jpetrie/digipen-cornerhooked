/*! ========================================================================

      @file    Input.cpp
      @author  jmp
      @brief   Implementation of input manager.

      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Input.h"

#include "Game.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const int  kBindingStringLen = 256;
  const int  kKeyboardBufLen   = 16;
};


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Input::Input(void)
/*! Default constructor.
*/
: mDIObject(0),
  mDIKeyboard(0),mDIMouse(0),
  mKeyDownInt(0),mLeftClickInt(0),mRightClickInt(0)
{
  ENFORCE_DX(::DirectInput8Create(::GetModuleHandle(0),DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&mDIObject,0))
            ("Could not initialize DirectInput.");
    
  nInitKeyboard();
  nInitMouse();
}

/*  ________________________________________________________________________ */
Input::~Input(void)
/*! Destructor.
*/
{
  // Release the mouse and keyboard.
  if(0 != mDIMouse)
  {
    mDIMouse->Unacquire();
    mDIMouse->Release();
    mDIMouse = 0;
  }
  if(0 != mDIKeyboard)
  {
    mDIKeyboard->Unacquire();
    mDIKeyboard->Release();
    mDIKeyboard = 0;
  }

  // Finally, release DI object.
  SAFE_RELEASE(mDIObject);
}

/*  ________________________________________________________________________ */
void Input::Update(void)
/*! Update all input devices.
*/
{
HWND                wind = Game::Get()->GetWindow()->GetHandle();
POINT               pos;
DIDEVICEOBJECTDATA  keyData[kKeyboardBufLen];
DWORD               elems;

  // Invoke keyboard interrupts.
  elems = kKeyboardBufLen;
  ENFORCE_DX(mDIKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),keyData,&elems,0))
            ("Buffered input read failed!");
  for(DWORD i = 0; i < elems; ++i) 
  {
    if(!!(keyData[i].dwData & 0x80) && 0 != mKeyDownInt)
      mKeyDownInt(keyData[i].dwOfs);
  }                                  
                                  
  // If we've lost the mouse, reacquire it (this updates the mouse state
  // buffer as well).
DIMOUSESTATE2  oldState = mMouseBuffer;

  if(mDIMouse->GetDeviceState(sizeof(mMouseBuffer),(LPVOID)&mMouseBuffer) == DIERR_INPUTLOST)
    mDIMouse->Acquire();
  
  // Update the fixed positions.
  ::GetCursorPos(&pos);
  if(!Game::Get()->GetWindow()->GetRenderer()->IsFullscreen())
    ::ScreenToClient(wind,&pos);
  mMouseX = pos.x;
  mMouseY = pos.y;
  mMouseZ = mMouseBuffer.lZ;

  // Invoke mouse interrupts.
  if(!!oldState.rgbButtons[kMB_Left] && !mMouseBuffer.rgbButtons[kMB_Left] &&
     0 != mLeftClickInt)
    mLeftClickInt(mMouseX,mMouseY,mMouseZ);
  if(!!oldState.rgbButtons[kMB_Right] && !mMouseBuffer.rgbButtons[kMB_Right] &&
     0 != mRightClickInt)
    mRightClickInt(mMouseX,mMouseY,mMouseZ);
}

/*  ________________________________________________________________________ */
bool Input::Key(int k) const
/*! Determine if a key is pressed.
    
    @param k  Code for the key to test (e.g., DIK_LSHIFT, et cetera).
    
    @return
    True if the key specified is pressed, false otherwise.
*/
{
  // Force the value to true or false.
  return (!!mKeyBuffer[k]);
}

/*  ________________________________________________________________________ */
bool Input::KeyFor(const std::string &action) const
/*! Determine if the key for a specified action is pressed.
    
    @param action  Name of the action to test.
    
    @return
    True if the key specified is pressed, false if the action does not exist
    or the key is not pressed.
*/
{
std::map< std::string,int >::const_iterator  it = mKeyBinds.find(action);

  // Does action even exist?
  if(it == mKeyBinds.end())
    return (false);
  
  // Then see if its pressed.
  return (Key(it->second));
}

/*  ________________________________________________________________________ */
bool Input::MouseButton(int b) const
/*! Determine if a mouse button is down.
    
    @param b  Code for the button to test.
    
    @return
    True if the button specified is down, false otherwise.
*/
{
  // Force the value to true or false.
  return (!!mMouseBuffer.rgbButtons[b]);
}

/*  ________________________________________________________________________ */
void Input::ReadInputBindings(const std::string &fn)
/*! Read input bindings from an .ini file.

    Any options not found in the .ini will be set to defaults.

    @param fn  The name of the .ini file.
*/
{
  // Camera controls.
  mKeyBinds["ConstrainX"] = ::GetPrivateProfileInt("Camera","ConstrainX",DIK_1,fn.c_str());
  mKeyBinds["ConstrainY"] = ::GetPrivateProfileInt("Camera","ConstrainY",DIK_2,fn.c_str());
  mKeyBinds["ConstrainZ"] = ::GetPrivateProfileInt("Camera","ConstrainZ",DIK_3,fn.c_str());
  mKeyBinds["ToggleRoll"] = ::GetPrivateProfileInt("Camera","ToggleRoll",DIK_LCONTROL,fn.c_str());
}

/*  ________________________________________________________________________ */
void Input::WriteInputBindings(const std::string &fn)
/*! Write input bindings to an .ini file.

    @param fn  The name of the .ini file.
*/
{
char  buf[kBindingStringLen];
  
  // The .ini routines provided by Windows do not seem to be able to write
  // integers to user-specified files, so we have to muck around with
  // conversions to strings, and whatnot.

  // Camera controls.
  ::sprintf(buf,"%d",mKeyBinds["ConstrainX"]);
  ::WritePrivateProfileString("Camera","ConstrainX",buf,fn.c_str());
  ::sprintf(buf,"%d",mKeyBinds["ConstrainY"]);
  ::WritePrivateProfileString("Camera","ConstrainY",buf,fn.c_str());
  ::sprintf(buf,"%d",mKeyBinds["ConstrainZ"]);
  ::WritePrivateProfileString("Camera","ConstrainZ",buf,fn.c_str());
  ::sprintf(buf,"%d",mKeyBinds["ToggleRoll"]);
  ::WritePrivateProfileString("Camera","ToggleRoll",buf,fn.c_str());
}

/*  ________________________________________________________________________ */
void Input::nInitKeyboard(void)
/*! Initialize and configure the keyboard.
*/
{
HWND        wind = Game::Get()->GetWindow()->GetHandle();
DIPROPDWORD prop;

  // Fill out property structure.
  prop.diph.dwSize       = sizeof(DIPROPDWORD);
  prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  prop.diph.dwObj        = 0;
  prop.diph.dwHow        = DIPH_DEVICE;
  prop.dwData            = kKeyboardBufLen;


  ENFORCE_DX(mDIObject->CreateDevice(GUID_SysKeyboard,&mDIKeyboard,0))
            ("Could not create keyboard device.");
  
  // Setup.
  ENFORCE_DX(mDIKeyboard->SetProperty(DIPROP_BUFFERSIZE,&prop.diph))
            ("Could not set buffered keyboard property.");
  ENFORCE_DX(mDIKeyboard->SetDataFormat(&c_dfDIKeyboard))
            ("Could not set keyboard data format.");
  
  // Finally set cooperative level and acquire the device.
  ENFORCE_DX(mDIKeyboard->SetCooperativeLevel(wind,DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))
            ("Could not set keyboard access rights.");
  ENFORCE_DX(mDIKeyboard->Acquire())
            ("Could not acquire the keyboard.");
}

/*  ________________________________________________________________________ */
void Input::nInitMouse(void)
/*! Initialize and configure the mouse.
*/
{
HWND       wind = Game::Get()->GetWindow()->GetHandle();
DIDEVCAPS  caps;
POINT      pos;

  ENFORCE_DX(mDIObject->CreateDevice(GUID_SysMouse,&mDIMouse,0))
            ("Could not create mouse device.");

  // This input device needs to use mouse data format.
  ENFORCE_DX(mDIMouse->SetDataFormat(&c_dfDIMouse2))
            ("Could not set mouse data format.");
  
  // Set cooperative level, as with keyboard, and acquire.
  ENFORCE_DX(mDIMouse->SetCooperativeLevel(wind,DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))
            ("Could not set mouse access rights.");
  ENFORCE_DX(mDIMouse->Acquire())
            ("Could not acquire the mouse.");

  // Get mouse capabilities.
  caps.dwSize = sizeof(DIDEVCAPS);
  ENFORCE_DX(mDIMouse->GetCapabilities(&caps))
            ("Could not probe mouse capabilities.");
  
  // Initialize the mouse coordinates.
  ::GetCursorPos(&pos);
  if(!Game::Get()->GetWindow()->GetRenderer()->IsFullscreen())
    ::ScreenToClient(wind,&pos);
  mMouseX = pos.x;
  mMouseY = pos.y;
}

/*  ________________________________________________________________________ */
char Input::GetKeyASCII(DWORD scancode)
/*! Convert a keyboard scancode to an ASCII value.

    @param scancode  The scancode to convert.
    
    @return
    If the scancode is convertable, the ASCII conversion is returned.
    Otherwise, 0 is returned.
*/
{
static HKL            layout = ::GetKeyboardLayout(0);
static unsigned char  state[256];
unsigned int          virt;
unsigned short        ascii;

   if(!::GetKeyboardState(state))
     return (0);
   
   // Mapping type 1 means translate scancode to virtual key code, and
   // don't bother with left/right versions of keys.
   virt = ::MapVirtualKeyEx(scancode,1,layout);
   
   // Convert to ASCII.
   if(0 == ::ToAsciiEx(virt,scancode,state,&ascii,0,layout))
    return (0);
   return (static_cast< char >(ascii & 0xFF));
}