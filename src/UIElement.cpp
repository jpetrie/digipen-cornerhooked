/*! ========================================================================

      @file    UIElement.cpp
      @author  jmp
      @brief   Implementation of UIElement.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIElement.h"

#include "Game.h"
#include "Window.h"

#include "Skybox.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  float  kTransitionMinElapsed = 0.020f;  
}


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIElement::UIElement(int x,int y,int w,int h,const std::string &tex,const std::string &caption,int textSize)
/*! Constructor.

    @param x         X coordinate.
    @param y         Y coordinate.
    @param w         Width of the element.
    @param h         Height of the element.
    @param tex       Texture of the element (empty for none).
    @param caption   Caption of the element.
    @param textSize  Caption or element text size.
*/
: mX(x),mY(y),
  mW(w),mH(h),
  mRealX(x),mRealY(y),
  mStall(0.0f),mRealStall(0.0f),
  mTrans(kNoTrans),mSpeed(1.0f),mAccumTime(0.0f),mTransSz(0),
  mInTransition(false),mIsEnabled(true),mIsFocused(false),mIsLogical(false),
  mIsSideCaption(false)
{ 
  // Grab the render device.
  mRenderer = Game::Get()->GetWindow()->GetRenderer(); 
  mCaption  = caption;
  
  mIsDead = false;
  
  mLabelR = new DXFont(Game::Get()->GetWindow()->GetRenderer(),"Verdana",textSize,DXFont::kFlagBold);
  mFontR  = new DXFont(Game::Get()->GetWindow()->GetRenderer(),"Verdana",textSize,0);
  mRectR  = new DXRect(Game::Get()->GetWindow()->GetRenderer(),
                       Skybox::UIBase(),
                       Skybox::UIBase(),
                       Skybox::UIBase(),
                       Skybox::UIBase(),
                       tex);
  
  // Now set up state functions.
  mEntranceStateID = DefineState(nEntranceUpdate,nEntranceEnter,nEntranceExit);
  mExitStateID     = DefineState(nExitUpdate,nExitEnter,nExitExit);
  mIdleStateID     = DefineState(nIdleUpdate,0,0);
  TransitionTo(mEntranceStateID);
}

/*  ________________________________________________________________________ */
UIElement::~UIElement(void)
/*! Destructor.
*/
{
  SAFE_DELETE(mLabelR);
  SAFE_DELETE(mFontR);
  SAFE_DELETE(mRectR);
}

/*  ________________________________________________________________________ */
void UIElement::SetTransitionProps(float stall,float speed,int trans,int transSz)
/*! Set transition properties.

    The transSz parameter control how big the transition thinks the element is.
    This is for fixing elements that are actually larger than they appear (such
    as the game's title graphics), usually so they don't transition off-screen
    too fast.
    
    @param stall    How many seconds the element waits to begin transition.
    @param speed    Speed scalar.
    @param trans    Transition type.
    @param transSz  Size of the element for transition purposes.
*/
{
  mStall  = mRealStall = stall;
  mSpeed  = speed;
  mTrans  = trans;
  if(transSz > 0)
    mTransSz = transSz;
}

/*  ________________________________________________________________________ */
void UIElement::Render(void)
/*! Render the element.
*/
{
  // If logical, don't render.
  if(mIsLogical)
    return;

  // Render the background.
  mRectR->DrawRect(mX,mY,mW,mH);
  
SIZE len = mFontR->GetStringWidth(mCaption);

  len.cx /= 2;
 // if(mIsEnabled)
 //   mFontR->DrawText(mX + (mW / 2 - len.cx),mY,kUIElem_TextColor,mCaption.c_str());
 // else
 //   mFontR->DrawText(mX + (mW / 2 - len.cx),mY,kUIElem_DisabledTextColor,mCaption.c_str());
  mFontR->DrawText(mX,mY,mW,mH,Skybox::UIText(),DT_LEFT,mCaption);
}

/*  ________________________________________________________________________ */
void UIElement::Reset(void)
/*! Make the element reset itself.
*/
{
  TransitionTo(mEntranceStateID);
}

/*  ________________________________________________________________________ */
void UIElement::Exit(void)
/*! Make the element exit the screen.
*/
{
  TransitionTo(mExitStateID);
}

/*  ________________________________________________________________________ */
int UIElement::InvokeCallback(UIElement::Event event)
/*! Invoke a callback for the specified event.
    
    @param event  Event code for the callback to invoke.
    
    @return
    The result from the callback, or zero if no callback was invoked.
*/
{
std::map< Event,Callback >::iterator  it = mCallbacks.find(event);

  if(it != mCallbacks.end())
    return (it->second());
  return (0);
}

/*  ________________________________________________________________________ */
void UIElement::InstallCallback(UIElement::Event event,UIElement::Callback cb)
/*! Install a callback for the specified event.

    This function will replace an existing callback for the event, if one
    exists.

    @param event  Event code for the callback to invoke.
    @param cb     The callback to invoke.
*/
{
  mCallbacks[event] = cb;
}

/*  ________________________________________________________________________ */
void UIElement::nEntranceEnter(StateMachine *sm,float /*elapsed*/)
/*! 
*/
{
UIElement *e = static_cast< UIElement* >(sm);
int        w = Game::Get()->GetWindow()->Width();
int        h = Game::Get()->GetWindow()->Height();

  // Initialize the transition.
  switch(e->mTrans)
  { 
    case kSlideFromLeft:
      e->mTarget       = e->mRealX;
      e->mX            = (e->mTransSz > 0 ? -e->mTransSz : -e->mW);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
    
    case kSlideFromRight:
      e->mTarget       = e->mRealX;
      e->mX            = (e->mTransSz > 0 ? w - (e->mW - e->mTransSz) : w);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
    
    case kSlideFromTop:
      e->mTarget       = e->mRealY;
      e->mY            = (e->mTransSz > 0 ? -e->mTransSz : -e->mH);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
      
    case kSlideFromBottom:
      e->mTarget       = e->mRealY;
      e->mY            = (e->mTransSz > 0 ? h - (e->mH - e->mTransSz) : h);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
      
    default:
    {
      // Nothing.
    }
  }
}

/*  ________________________________________________________________________ */
void UIElement::nEntranceExit(StateMachine *sm,float elapsed)
/*! Entrance state clean up function.

    @param sm       The statemachine (convertable to UIElement).
    @param elapsed  Always zero for clean up functions.
*/
{
  (sm);
  (elapsed);
}

/*  ________________________________________________________________________ */
void UIElement::nEntranceUpdate(StateMachine *sm,float elapsed)
/*! Entrance state update function.

    This function advances element transition animations.
    
    @param sm       The statemachine (convertable to UIElement).
    @param elapsed  The elapsed time since last update.
*/
{
UIElement *e = static_cast< UIElement* >(sm);
float      delta;
float      dist;

  // Are we dead? Don't do anything.
  if(e->mIsDead)
    return;

  // Should we stall?
  if(e->mStall > 0.0f)
  {
    e->mStall -= elapsed;
    return;
  }
  
  // Update accumulator, reset if we can step the transition.
  e->mAccumTime += elapsed;
  if(e->mAccumTime <= kTransitionMinElapsed)
    return;
  else
    e->mAccumTime = 0.0f;

  // Handle transition.
  switch(e->mTrans)
  {
    case kSlideFromLeft:
      dist  = static_cast< float >(abs(e->mX - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mX = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mX += static_cast< int >(delta);
      break;
    
    case kSlideFromRight:
      dist  = static_cast< float >(abs(e->mX - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mX = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mX -= static_cast< int >(delta);
      break;
    
    case kSlideFromTop:
      dist  = static_cast< float >(abs(e->mY - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mY = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mY += static_cast< int >(delta);
      break;
      
    case kSlideFromBottom:
      dist  = static_cast< float >(abs(e->mY - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mY = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mY -= static_cast< int >(delta);
      break;
      
    default:
    {
      // Just go idle.
      e->TransitionTo(e->mIdleStateID);
    }
  }
}

/*  ________________________________________________________________________ */
void UIElement::nExitEnter(StateMachine *sm,float /*elapsed*/)
/*! 
*/
{
UIElement *e = static_cast< UIElement* >(sm);
int        w = Game::Get()->GetWindow()->Width();
int        h = Game::Get()->GetWindow()->Height();

  // Initialize the transition.
  switch(e->mTrans)
  { 
    case kSlideFromLeft:
      e->mTarget       = (e->mTransSz > 0 ? -e->mTransSz : -e->mW);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
    
    case kSlideFromRight:
      e->mTarget       = (e->mTransSz > 0 ? w - (e->mW - e->mTransSz) : w);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
    
    case kSlideFromTop:
      e->mTarget       = (e->mTransSz > 0 ? -e->mTransSz : -e->mH);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
    
    case kSlideFromBottom:
      e->mTarget       = (e->mTransSz > 0 ? h - (e->mH - e->mTransSz) : h);
      e->mStall        = e->mRealStall;
      e->mInTransition = true;
      e->mAccumTime    = 0.0f;
      break;
      
    default:
    {
      // Nothing.
    }
  }
}

/*  ________________________________________________________________________ */
void UIElement::nExitExit(StateMachine * /*sm*/,float /*elapsed*/)
/*! 
*/
{

}

/*  ________________________________________________________________________ */
void UIElement::nExitUpdate(StateMachine *sm,float elapsed)
/*! Exit state update function.

    This function advances element transition animations.
    
    @param sm       The statemachine (convertable to UIElement).
    @param elapsed  The elapsed time since last update.
*/
{
UIElement *e = static_cast< UIElement* >(sm);
float      delta;
float      dist;

  // Should we stall?
  if(e->mStall > 0.0f)
  {
    e->mStall -= elapsed;
    return;
  }
  
  // Update accumulator, reset if we can step the transition.
  e->mAccumTime += elapsed;
  if(e->mAccumTime <= kTransitionMinElapsed)
    return;
  else
    e->mAccumTime = 0.0f;

  // Handle transition.
  switch(e->mTrans)
  {
    case kSlideFromLeft:
      dist  = static_cast< float >(abs(e->mX - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mX = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mX -= static_cast< int >(delta);
      break;
    
    case kSlideFromRight:
      dist  = static_cast< float >(abs(e->mX - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mX = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mX += static_cast< int >(delta);
      break;
    
    case kSlideFromTop:
      dist  = static_cast< float >(abs(e->mY - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mY = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mY -= static_cast< int >(delta);
      break;
      
    case kSlideFromBottom:
      dist  = static_cast< float >(abs(e->mY - e->mTarget));
      delta = e->mSpeed * (dist / 2.0f);
      if(delta <= 1.0f)
      {
        e->mY = e->mTarget;
        e->TransitionTo(e->mIdleStateID);
      }
      else
        e->mY += static_cast< int >(delta);
      break;
       
    default:
    {
      // Just go idle.
      e->TransitionTo(e->mIdleStateID);
    }
  }
}

/*  ________________________________________________________________________ */
void UIElement::nIdleUpdate(StateMachine *sm,float /*elapsed*/)
/*! 
*/
{
UIElement *e = static_cast< UIElement* >(sm);
  
  e->mInTransition = false;
}  