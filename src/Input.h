/*! ========================================================================

      @file    Input.h
      @author  jmp
      @brief   Interface to input manager.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _INPUT_H_
#define _INPUT_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

// mouse codes
const int kMB_Left   = 0;
const int kMB_Right  = 1;
const int kMB_Middle = 2;


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Input
/*! DirectInput manager class.
*/
{
  public:
    // typedefs
    typedef void (*KeyDownInt)(int);
    typedef void (*MouseClickInt)(int,int,int);
  
    // ct and dt
    Input(void);
    ~Input(void);

    // update
    void Update(void);
    
    // keyboard accessors
    bool Key(int k) const;
    bool KeyFor(const std::string &action) const;
    
    // mouse accessors
    long MouseX(void) const      { return (mMouseX); }
    long MouseY(void) const      { return (mMouseY); }
    long MouseZ(void) const      { return (mMouseZ); }
    long MouseXDelta(void) const { return (mMouseBuffer.lX); }
    long MouseYDelta(void) const { return (mMouseBuffer.lY); }
    long MouseZDelta(void) const { return (mMouseBuffer.lZ); }
    bool MouseButton(int b) const;
    
    // bindings
    void ReadInputBindings(const std::string &fn);
    void WriteInputBindings(const std::string &fn);
    
    // scancode-to-ascii mapping
    static char GetKeyASCII(DWORD scancode);
    
    // interrupt handlers
    void InstallKeyDownHandler(KeyDownInt k)       { mKeyDownInt = k; }
    void InstallLeftClickHandler(MouseClickInt l)  { mLeftClickInt = l; }
    void InstallRightClickHandler(MouseClickInt r) { mRightClickInt = r; }
    
  private:
    // init helpers
    void nInitKeyboard(void); 
    void nInitMouse(void);
    
    
    // data members
    LPDIRECTINPUT8        mDIObject;    //!< DI8 interface object.
    LPDIRECTINPUTDEVICE8  mDIKeyboard;  //!< Keyboard object.  
    LPDIRECTINPUTDEVICE8  mDIMouse;     //!< Mouse object.
    
    //LPDIRECT3DSURFACE9    mCursorSurface;  //!< Mouse cursor surface.
  
    char           mKeyBuffer[256];  //!< Keyboard input buffer.
    DIMOUSESTATE2  mMouseBuffer;     //!< Mouse state buffer.
    long           mMouseX;          //!< Fixed mouse X position.
    long           mMouseY;          //!< Fixed mouse Y position.
    long           mMouseZ;          //!< Fixed mouse Z position.
    
    std::map< std::string,int >  mKeyBinds;  //!< Maps action names to key bindings.
    
    KeyDownInt     mKeyDownInt;
    MouseClickInt  mLeftClickInt;
    MouseClickInt  mRightClickInt;
};


#endif  /* _INPUTMGR_H_ */