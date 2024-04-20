/*! ========================================================================

      @file    UIPanel.cpp
      @author  jmp
      @brief   Implementation of UIPanel.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== 
 */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "Window.h"

#include "UIPanel.h"
#include "UIListbox.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIPanel::UIPanel(int x,int y,int w,int h)
/*! Constructor.
*/
: UIElement(x,y,w,h,"","")
{
  // Override background color.
  SAFE_DELETE(mRectR);
  mRectR = new DXRect(Game::Get()->GetWindow()->GetRenderer(),
                      kUIElem_PanelColor,kUIElem_PanelColor,
                      kUIElem_PanelColor,kUIElem_PanelColor,"");
}

/*  ________________________________________________________________________ */
UIPanel::~UIPanel(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
void UIPanel::AddElement(UIElement *e)
/*! Adds an element to the panel.

    Note that the UI element receives no name, therefor cannot be looked up
    again once added.
    
    @param e  The UI element to add.
*/
{
  mUnnamedElem.push_back(e);
  mAllElements.push_back(e);
}

/*  ________________________________________________________________________ */
void UIPanel::AddElement(const std::string &name,UIElement *e)
/*! Adds an element to the panel.

    @param name  The name to associate with the element.  
    @param e     The UI element to add.
*/
{
  mNamedElem[name] = e;
  mAllElements.push_back(e);
}

/*  ________________________________________________________________________ */
void UIPanel::Render(void)
/*! Render the panel.
*/
{
TElemList::iterator  it = mAllElements.begin();

  // Render the panel.
  UIElement::Render();

  // Render all children.
  while(it != mAllElements.end())
  {
  int  tx = (*it)->X();
  int  ty = (*it)->Y();
  
    (*it)->X(tx + mX);
    (*it)->Y(ty + mY);
	if ((*it)->GetType() == kListbox)
      scast<UIListbox*>((*it))->Render(tx,ty);
	else
      (*it)->Render();
    (*it)->X(tx);
    (*it)->Y(ty);
    ++it;
  }
}

/*  ________________________________________________________________________ */
UIElement* UIPanel::GetElement(const std::string &name)
/*! Get a named element from the panel.

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
bool UIPanel::CheckLeftClick(int x,int y,UIElement *&hit)
/*! Check a left click.

    @param  x    X location of the click.
    @param  y    Y location of the click.
    @param  hit  On output, the element that took the click.

    @return
    True if an element within the panel, or the panel itself, took the click.
    False otherwise. This function will always return true.
*/
{
TElemList::iterator  it  = mAllElements.begin();

  x -= mX;
  y -= mY;

  while(it != mAllElements.end())
  {
    SIZE  offset = (*it)->GetCaptionOffset();
	int   lbOffset = ((*it)->GetType() == UIElement::kListbox) ? kUIElem_LabelHeight : 0 ;
  
    if(x >= (*it)->X() + offset.cx && x <= ((*it)->X() + offset.cx + (*it)->Width()) &&
       y >= (*it)->Y() + lbOffset && y <= ((*it)->Y() + (*it)->Height() + lbOffset))
    { 
      // Invoke the callback.
      hit = *it;
      if(hit->IsEnabled())
      {
        if (hit->GetType() == UIElement::kEditText && !scast<UIEditText*>(hit)->IsReadOnly())
		{
          hit->CheckLeftClick(x,y);
		  //// Callback called from UIScreen::CheckLeftClick ////
          //hit->InvokeCallback(UIElement::kLeftClick);
		}
//		else if (hit->GetType() == UIElement::kListbox)
//		{
//          hit->CheckLeftClick(x+mX, y+mY);
//          hit->InvokeCallback(UIElement::kLeftClick);
//		}
		else if (hit->GetType() != UIElement::kEditText)
		{
          hit->CheckLeftClick(x,y);
		  //// Callback called from UIScreen::CheckLeftClick ////
          //hit->InvokeCallback(UIElement::kLeftClick);
		}
      }
      return (true);
    }
    ++it;
  }

  // The panel takes one for the team.
  hit = this;
//  InvokeCallback(UIElement::kLeftClick);
  return (false);
}