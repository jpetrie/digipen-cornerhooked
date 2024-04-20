/*! ========================================================================

      @file    UIListbox.cpp
      @author  jmp
      @brief   Implementation of UIListbox.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIListbox.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIListbox::UIListbox(int x,int y,int w,int h)
/*! Constructor.

    @param x  The X coordinate of the element.
    @param y  The Y coordinate of the element.
    @param w  The width of the element.
    @param h  The height of the element.
*/
: UIElement(x,y,w,h,"",""),
  mSelectedItem(-1)
{
 // mW -= (kUIElem_ScrollbarW + kUIElem_ScrollbarOffset);
  mScrollbar = new UIScrollbar(mX + mW + kUIElem_ScrollbarOffset,mY,kUIElem_ScrollbarW,mH);
  mScrollbar->SetMax(0);
}

/*  ________________________________________________________________________ */
UIListbox::~UIListbox(void)
/*! Destructor.
*/
{
  SAFE_DELETE(mScrollbar);
}

/*  ________________________________________________________________________ */
int UIListbox::AddItem(const std::string &text)
/*! Add an item to the list box.
*/
{
  mItems.push_back(text);
//  int min = mH / kUIElem_LineHeight;
//  int max = (mItems.size() > min) ? mItems.size() : min;
  int max = mItems.size() - mH / kUIElem_LineHeight ;
  if (max < 0)
    max = 0 ;
  mScrollbar->SetMax(max);
  return (static_cast< int >(mItems.size()) - 1);
}

/*  ________________________________________________________________________ */
void UIListbox::ClearItems(void)
/*! Clears all items in the list box.
*/
{
  mItems.clear();
  mScrollbar->SetMax(0);
}

/*  ________________________________________________________________________ */
void UIListbox::SetSelected(int idx)
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
bool UIListbox::CheckLeftClick(int x,int y)
/*! 
*/
{
  // Select the item.
unsigned idx = (y - (mY + kUIElem_LabelHeight)) / kUIElem_LineHeight;
  
  if(x > (mX + (mW - kUIElem_ScrollbarW)))
    mScrollbar->CheckLeftClick(x,y);
  else
    SetSelected(idx);
  return (true);
}

/*  ________________________________________________________________________ */
void UIListbox::Render(void)
/*! Render the list box.
*/
{
  // Render the label.
  mLabelR->DrawText(mX,mY,mW,mH,kUIElem_TextColor,DT_LEFT,mCaption.c_str());
  
  // Render the background.
  mRectR->DrawRect(mX,mY + kUIElem_LabelHeight,mW,mH);

  // Calculate how many items are going to fit into the visible
  // area of the listbox.
  int  nitems = mH / kUIElem_LineHeight;
  int  start  = mScrollbar->GetCur();

  if(nitems > static_cast< int >(mItems.size()))
    nitems = static_cast< int >(mItems.size());
  if(start > static_cast< int >(mItems.size()) - nitems)
    start = static_cast< int >(mItems.size()) - nitems;
  if(start < 0)
    start = 0;

  // Then render each element.
  for(int i = 0; i < nitems; ++i)
  {
    mRectR->DrawRect(mX,mY + kUIElem_LabelHeight + (kUIElem_LineHeight * i),mW - (kUIElem_ScrollbarW + kUIElem_ScrollbarOffset),kUIElem_LineHeight);
    
    if(i == mSelectedItem)
      mFontR->DrawText(mX,mY + kUIElem_LabelHeight + (kUIElem_LineHeight * i),mW - (kUIElem_ScrollbarW + kUIElem_ScrollbarOffset),mH,D3DCOLOR_ARGB(255,255,0,0),DT_LEFT,mItems[i + start].c_str());
    else///@todo a better way of rendering selected is needed
      mFontR->DrawText(mX,mY + kUIElem_LabelHeight + (kUIElem_LineHeight * i),mW - (kUIElem_ScrollbarW + kUIElem_ScrollbarOffset),mH,D3DCOLOR_ARGB(255,255,255,255),DT_LEFT,mItems[i + start].c_str());
  }
}

/*  ________________________________________________________________________ */
void UIListbox::Render(int oldX,int oldY)
/*! Overload to avoid coordinate problems with scrollbars.
*/
{
  mScrollbar->X(mX + mW - kUIElem_ScrollbarW);
  mScrollbar->Y(mY + kUIElem_LabelHeight);
  mScrollbar->Render();
  mScrollbar->X(oldX + mW - kUIElem_ScrollbarW);
  mScrollbar->Y(oldY + kUIElem_LabelHeight);

  // The rest is in the normal Render
  Render();
}