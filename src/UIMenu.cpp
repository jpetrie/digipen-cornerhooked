/*! ========================================================================

      @file    UIMenu.cpp
      @author  jmp
      @brief   Implementation of UIMenu.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIMenu.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIMenu::UIMenu(int x,int y,int w,int h)
/*! Constructor.

    @param x  The X coordinate of the element.
    @param y  The Y coordinate of the element.
    @param w  The width of the element.
    @param h  The height of the element.
*/
: UIElement(x,y,w,h,"",""),
  mSelectedItem(0)
{
}

/*  ________________________________________________________________________ */
UIMenu::~UIMenu(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
int UIMenu::AddItem(const std::string &text)
/*! Add an item to the menu.
*/
{
  mItems.push_back(text);
  return (static_cast< int >(mItems.size()) - 1);
}

/*  ________________________________________________________________________ */
void UIMenu::ClearItems(void)
/*! Clears all items in the menu.
*/
{
  mItems.clear();
}

/*  ________________________________________________________________________ */
void UIMenu::SetSelected(int idx)
/*! Set the selected item index.

    If idx is -1 or out of range, no items will be selected.
 
    @param idx  Index of the item to select.
*/
{
  if(idx < 0 || idx >= static_cast< int >(mItems.size()))
    idx = -1;
  mSelectedItem = idx;
}

/*  ________________________________________________________________________ */
bool UIMenu::CheckLeftClick(int /*x*/,int /*y*/)
/*! 
*/
{
  // Select the item.
//unsigned idx = (y - (mY + kUIElem_LabelHeight)) / kUIElem_LineHeight;
  

  return (true);
}

/*  ________________________________________________________________________ */
void UIMenu::Render(void)
/*! Render the list box.
*/
{
SIZE  offset = { 0,0 };

  if(mIsSideCaption)
  {
    offset = mLabelR->GetStringWidth(mCaption);
    mLabelR->DrawText(mX,mY,offset.cx,kUIElem_LabelHeight,kUIElem_LabelColor,DT_CENTER | DT_VCENTER,mCaption);
  }

  // Render the background.
  mRectR->DrawRect(mX + offset.cx,mY,mW,mH);
  
  if(mIsFocused)
    mFontR->DrawText(mX + offset.cx,mY,mW,mH,kUIElem_TextColor,DT_LEFT | DT_VCENTER,mItems[mSelectedItem]);
  else
    mFontR->DrawText(mX + offset.cx,mY,mW,mH,0xFFFF00FF,DT_LEFT | DT_VCENTER,mItems[mSelectedItem]);
}


