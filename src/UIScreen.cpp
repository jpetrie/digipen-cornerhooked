/*! ========================================================================

      @file    UIScreen.cpp
      @author  jmp
      @brief   Implementation of UIScreen.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIScreen.h"
#include "UIElement.h"

#include "UIEditText.h"
#include "UIPanel.h"

#include "Input.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIScreen::UIScreen(void)
/*! Default constructor.
*/
: mFocused(0)
{
  mExitCallback = 0;
}

/*  ________________________________________________________________________ */
UIScreen::~UIScreen(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
void UIScreen::AddElement(UIElement *e)
/*! Add an element to the screen.

    Note that the UI element receives no name, therefor cannot be looked up
    again once added.
    
    @param e  The UI element to add.
*/
{
  mUnnamedElem.push_back(e);
  mAllElements.push_back(e);
}

/*  ________________________________________________________________________ */
void UIScreen::AddElement(const std::string &name,UIElement *e)
/*! Add an element to the screen.

    @param name  The name to associate with the element.  
    @param e     The UI element to add.
*/
{
  mNamedElem[name] = e;
  mAllElements.push_back(e);
}

/*  ________________________________________________________________________ */
UIElement* UIScreen::GetElement(const std::string &name)
/*! Get a named element from the screen.

    @param name  The name of the element.
    
    @return
    A pointer to the element, to null if it was not found.
*/
{
TElemMap::iterator it = mNamedElem.find(name);

  if(it != mNamedElem.end())
    return (it->second);
  return (0);
}

/*  ________________________________________________________________________ */
bool UIScreen::CheckLeftClick(int x,int y)
/*! 
*/
{
TElemList::iterator  it  = mAllElements.begin();

  while(it != mAllElements.end())
  {
  SIZE  offset = (*it)->GetCaptionOffset();
  
    if(x >= (*it)->X() + offset.cx && x <= ((*it)->X() + offset.cx + (*it)->Width()) &&
       y >= (*it)->Y() && y <= ((*it)->Y() + (*it)->Height()))
    {
      // If disabled, you can't click.
      if(!(*it)->IsEnabled() || (*it)->GetType() == UIElement::kGraphic)
      {
        ++it;
        continue;
      }
	  // For EditTexts, you can't click if ReadOnly
	  if((*it)->GetType() == UIElement::kEditText && static_cast<UIEditText*>(*it)->IsReadOnly())
	  {
        ++it;
        continue;
	  }
      
      // If the element is a panel, check it children.
      if((*it)->GetType() == UIElement::kPanel)
      {
      UIElement *hit = 0;
      
        ((UIPanel*)(*it))->CheckLeftClick(x,y,hit);
        if(0 != hit && !hit->IsEnabled())
        {
          // Can't click disabled stuff.
          return (true);
        }
        
        // If the clicked element can take focus, give it the focus.
        // Otherwise unfocus the screen.
        if(0 != hit && (hit->GetType() == UIElement::kEditText || hit->GetType() == UIElement::kMenu))
        {
          if(0 != mFocused)
          {
            mFocused->Focus(false);
            mFocused->InvokeCallback(UIElement::kLoseFocus);
          }
          mFocused = hit;
          mFocused->Focus(true);
          mFocused->InvokeCallback(UIElement::kTakeFocus);
        }
        else
        {
          if(0 != mFocused)
          {
            mFocused->Focus(false);
            mFocused->InvokeCallback(UIElement::kLoseFocus);
          }
          mFocused = 0;
        }
          
		hit->InvokeCallback(UIElement::kLeftClick);
        return (true);
      }
      
      // If the clicked element can take focus, give it the focus.
      // Otherwise unfocus the screen.
      if((*it)->GetType() == UIElement::kEditText ||
         (*it)->GetType() == UIElement::kMenu)
      {
        if(0 != mFocused)
        {
          mFocused->Focus(false);
          mFocused->InvokeCallback(UIElement::kLoseFocus);
        }
        mFocused = *it;
        mFocused->Focus(true);
        mFocused->InvokeCallback(UIElement::kTakeFocus);
      }
      else
      {
        if(0 != mFocused)
        {
          mFocused->Focus(false);
          mFocused->InvokeCallback(UIElement::kLoseFocus);
        }
        mFocused = 0;
      }
          
      // And invoke the callback.
      (*it)->InvokeCallback(UIElement::kLeftClick);
      return (true);
    }
    ++it;
  }
  
  return (false);
}

/*  ________________________________________________________________________ */
bool UIScreen::CheckKeyDown(int key)
/*! Handle key down event.
*/
{
  // If something is focused, it takes the keystroke.
  if(0 != mFocused)
    if(mFocused->CheckKeyDown(key))
      return (true);
    else
    {
      // If the focused item was an edit box, and it didn't take the return,
      // we should unfocus it.
      mFocused->Focus(false);
      mFocused = 0;
      return (true);
    }
  
  return (false);
}

/*  ________________________________________________________________________ */
void UIScreen::Update(int /*x*/,int /*y*/,float elapsed)
/*! Update the screen.
*/
{
TElemList::iterator  it = mAllElements.begin();
bool                 callExitCB = true;

  while(it != mAllElements.end())
  {
    // Update the element.
    (*it)->Update(elapsed);
    if((*it)->IsInTransition())
      callExitCB = false;
    ++it;
  }
  
  if(callExitCB && 0 != mExitCallback)
  {
    mExitCallback();
    mExitCallback = 0;
  }
}

/*  ________________________________________________________________________ */
void UIScreen::Render(void)
/*! Render the screen.
*/
{
TElemList::iterator  it = mAllElements.begin();

  while(it != mAllElements.end())
  {
    (*it)->Render();
    ++it;
  }
}

/*  ________________________________________________________________________ */
void UIScreen::Reset(void)
/*! Resets the screen.
*/
{
TElemList::iterator  it = mAllElements.begin();

  while(it != mAllElements.end())
  {
    (*it)->Reset();
    ++it;
  }
}

/*  ________________________________________________________________________ */
void UIScreen::Exit(UIScreen::ExitFinishedCB cb)
/*! 
*/
{
TElemList::iterator  it = mAllElements.begin();

  mExitCallback = cb;
  while(it != mAllElements.end())
  {
    (*it)->Exit();
    ++it;
  }
}

/*  ________________________________________________________________________ */
void UIScreen::Focus(UIElement *pFocused)
/*! Set which item on the screen has the focus
	@param  pFocused  The new element to have the focus
*/
{
	if (pFocused == mFocused)
		return ;

	if (mFocused != 0)
	{
		mFocused->Focus( false ) ;
	    mFocused->InvokeCallback(UIElement::kLoseFocus);
	}

	mFocused = pFocused ;
	if (mFocused != 0)
	{
		mFocused->Focus( true ) ;
		mFocused->InvokeCallback(UIElement::kTakeFocus);
	}
}
