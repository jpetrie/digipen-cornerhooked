/*! ========================================================================

      @file    UIElement.h
      @author  jmp
      @brief   Interface to UIElement class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_ELEMENT_H_
#define _UI_ELEMENT_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"

#include "DXFont.h"
#include "DXRect.h"

#include "StateMachine.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// sizes
const int  kUIElem_MinHeight       = 10;
const int  kUIElem_MinWidth        = 10;
const int  kUIElem_LabelHeight     = 20;
const int  kUIElem_LineHeight      = 20;
const int  kUIElem_DefaultTextSize = 15;

// shadowing offsets
const int  kUIElem_ShadowOffX = 1;
const int  kUIElem_ShadowOffY = 1;

// colors
const DWORD kUIElem_BaseColor         = D3DCOLOR_ARGB(100,50,50,255);
const DWORD kUIElem_TextColor         = D3DCOLOR_ARGB(255,255,255,255);
const DWORD kUIElem_LabelColor        = D3DCOLOR_ARGB(255,200,200,255);
const DWORD kUIElem_DisabledTextColor = D3DCOLOR_ARGB(255,127,127,127);
const DWORD kUIElem_PanelColor        = D3DCOLOR_ARGB(50,50,50,255);


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIElement : public StateMachine
/*! Base class for all UI elements.
*/
{
  public:
    typedef int (*Callback)(void);
    
    enum Type
    //! Constants for element type.
    {
      kUntyped = 0,          //!< An untyped element.
      kGraphic,              //!< A graphic.
      kPushButton,           //!< A push button.
      kToggleButton,         //!< A toggle button.
      kEditText,
      kSlider,               //!< Unused.
      kColorSlider,          //!< Unused.
      kPanel,                //!< A panel.
      kPowerMeter,           //!< A shot power meter.
      kListbox,              //!< A list box.
      kScrollbar,            //!< A scroll bar.
      kMenu                  //!< A menu (combo-box style).
    };
    
    enum Transition
    {
      kNoTrans = 0,     //!< No transition.
      kSlideFromLeft,   //!< Slide in from the left of the screen.
      kSlideFromRight,  //!< Slide in from the right of the screen.
      kSlideFromTop,    //!< Slide in from the top of the screen.
      kSlideFromBottom  //!< Slide in from the bottom of the screen.
    };
    
    enum Event
    //! Constants for callback events.
    {
      kLeftClick,
      kRightClick,
      kTakeFocus,
      kLoseFocus
    };
    
    // ct and dt
             UIElement(int x,int y,int w,int h,const std::string &tex,const std::string &caption,int textSize = kUIElem_DefaultTextSize);
    virtual ~UIElement(void);
    
    // accessors
    virtual Type GetType(void) const = 0;
    int          X(void) const              { return (mX); }
    int          Y(void) const              { return (mY); }
    int          Width(void) const          { return (mW); }
    int          Height(void) const         { return (mH); }
    bool         IsInTransition(void) const { return (mInTransition); }
    bool         IsEnabled(void) const      { return (mIsEnabled /*&& !mInTransition*/); }
    bool         IsDead(void) const         { return (mIsDead); }
    bool         IsFocused(void) const      { return (mIsFocused); }
    bool         IsLogical(void) const      { return (mIsLogical); }
    bool         IsSideCaption(void) const  { return (mIsSideCaption); }
    
    // manipulators
    void X(int x)             { mX = x; }
    void Y(int y)             { mY = y; }
    void Width(int w)         { mW = (w > kUIElem_MinWidth) ? w : kUIElem_MinWidth; }
    void Height(int h)        { mH = (h > kUIElem_MinHeight) ? h : kUIElem_MinHeight; }
    void Enable(bool e)       { mIsEnabled = e; }
    void Die(bool d)          { mIsDead = d; }
    void Focus(bool f)        { mIsFocused = f; }
    void Logical(bool l)      { mIsLogical = l; }
    void SideCaption(bool sc) { mIsSideCaption = sc; }
	void RectTexture(const std::string &tex) { mRectR->SetTexture(tex); }
    
    // transitions
    void SetTransitionProps(float stall,float speed,int trans,int transSz = 0);
    
    // caption / label
    std::string  GetCaption(void) const           { return (mCaption); }
    void         SetCaption(const std::string &c) { mCaption = c; }
	SIZE         GetCaptionOffset(void) const     { SIZE sz = { 0,0 }; return (mIsSideCaption ? mLabelR->GetStringWidth(mCaption) : sz); }
    
    // render
    virtual void Render(void);
    
    // control
    void Reset(void);
    void Exit(void);
    
    // events
    virtual bool CheckLeftClick(int /*x*/,int /*y*/) { return (false); }
    virtual bool CheckKeyDown(int /*key*/)           { return (false); }
    
    // callbacks
    int  InvokeCallback(Event event);
    void InstallCallback(Event event,Callback cb);
    
    
  protected:
    // state functions
    static void nEntranceEnter(StateMachine *sm,float elapsed);
    static void nEntranceExit(StateMachine *sm,float elapsed);
    static void nEntranceUpdate(StateMachine *sm,float elapsed);
    static void nExitEnter(StateMachine *sm,float elapsed);
    static void nExitExit(StateMachine *sm,float elapsed);
    static void nExitUpdate(StateMachine *sm,float elapsed);
    static void nIdleUpdate(StateMachine *sm,float elapsed);
    
    
    // data members
    Graphics::Renderer *mRenderer;  //!< General graphics renderer.
    DXFont             *mLabelR;    //!< Label font renderer object.
    DXFont             *mFontR;     //!< Font renderer object.
    DXRect             *mRectR;     //!< Rect renderer object. 
    
    int    mRealX;  //!< Stored Y coordinate.
    int    mRealY;  //!< Stored X coordinate.
    int    mX;      //!< Current screen X coordinate.
    int    mY;      //!< Current screen Y coordinate.
    int    mW;      //!< Width of element.
    int    mH;      //!< Height of element.
    
    int    mTrans;      //!< Transition type.
    float  mSpeed;      //!< Transition speed scalar.
    float  mAccumTime;  //!< Accumulated transition time.
    int    mTarget;     //!< Target transition X or Y value (as appropriate).
    int    mTransSz;    //!< Size for transition (optional, defaults to actual size).
   
    float  mStall;      //!< Current stall time.
    float  mRealStall;  //!< Desired stall time (when reset);

    bool   mInTransition;   //!< True if transitioning, and therefore unusable.
    bool   mIsEnabled;      //!< True if enabled.
    bool   mIsDead;         //!< True if dead (will not go to entrance transition).
    bool   mIsFocused;      //!< True if focused.
    bool   mIsLogical;      //!< True if container is "logical" (not generally rendered).
    bool   mIsSideCaption;  //!< True if the caption should be rendered on the left of the element.
    
    int  mEntranceStateID;
    int  mExitStateID;
    int  mIdleStateID;
    
    std::string  mCaption;  //!< Text caption.
    
    std::map< Event,Callback >  mCallbacks;  //!< Action callbacks.
};


#endif  /* _UI_ELEMENT_H_ */