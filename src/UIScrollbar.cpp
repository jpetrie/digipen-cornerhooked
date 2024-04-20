/*! ========================================================================

      @file    UIScrollbar.cpp
      @author  jmp
      @brief   Implementation of UIScrollbar.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIScrollbar.h"

#include "Game.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIScrollbar::UIScrollbar(int x,int y,int w,int h,float pos)
/*! Constructor.

    @param x    The X coordinate of the element.
    @param y    The Y coordinate of the element.
    @param w    The width of the element.
    @param h    The height of the element.
    @param pos  The initial position of the scrollbar (0.0 to 1.0).
*/
: UIElement(x,y,w,h,"",""),
  mPosition(pos),
  mMin(0),mMax(100),mCur(0),mStep(1)
{
  mThumbUp = new DXRect(Game::Get()->GetWindow()->GetRenderer(),
                        kUIElem_BaseColor,
                        kUIElem_BaseColor,
                        kUIElem_BaseColor,
                        kUIElem_BaseColor,
                        "data/misc/thumb_up.png");
  mThumbDn = new DXRect(Game::Get()->GetWindow()->GetRenderer(),
                        kUIElem_BaseColor,
                        kUIElem_BaseColor,
                        kUIElem_BaseColor,
                        kUIElem_BaseColor,
                        "data/misc/thumb_dn.png");
  mThumb   = new DXRect(Game::Get()->GetWindow()->GetRenderer(),
                        D3DCOLOR_ARGB(255,255,255,255),
                        D3DCOLOR_ARGB(255,255,255,255),
                        D3DCOLOR_ARGB(255,255,255,255),
                        D3DCOLOR_ARGB(255,255,255,255));
}

/*  ________________________________________________________________________ */
UIScrollbar::~UIScrollbar(void)
/*! Destructor.
*/
{
  SAFE_DELETE(mThumb);
  SAFE_DELETE(mThumbUp);
  SAFE_DELETE(mThumbDn);
}

/*  ________________________________________________________________________ */
void UIScrollbar::StepUp(unsigned int step)
/*! Step the scrollbar up once (towards minimum value).
	@param  step  Step size
*/
{
  ASSERT(mCur >= mMin);
  ASSERT(mCur <= mMax);

  if(mMax - mMin <= 0)
    return ;

  if(mCur - scast< signed int >(step) >= mMin)
    mCur -= scast< signed int >(step);
  else
    mCur = mMin;

  mPosition = scast< float >(mCur) / scast< float >(mMax);
}

/*  ________________________________________________________________________ */
void UIScrollbar::StepDn(unsigned int step)
/*! Step the scrollbar down once (towards maximum value).
	@param  step  Step size
*/
{
  ASSERT(mCur >= mMin);
  ASSERT(mCur <= mMax);

  if(mMax - mMin <= 0)
    return ;

  if(mCur + scast< signed int >(step) <= mMax)
    mCur += scast< signed int >(step);
  else
    mCur = mMax;

  mPosition = scast< float >(mCur) / scast< float >(mMax);
}

/*  ________________________________________________________________________ */
bool UIScrollbar::CheckLeftClick(int /*x*/,int y)
/*! 
*/
{
  if(y < mY + kUIElem_ScrollThumbH)
    StepUp();                                     // Head button
  else if(y < mY + kUIElem_ScrollThumbH + mPosition * (mH - 3*kUIElem_ScrollThumbH))
    StepUp(5);                                    // Upper-half of main bar
  else if(y > mY + mH - kUIElem_ScrollThumbH)
    StepDn();                                     // Tail button
  else if(y > mY + 2*kUIElem_ScrollThumbH + mPosition * (mH - 4*kUIElem_ScrollThumbH))
    StepDn(5);                                    // Lower-half of main bar
  return (true);
}

/*  ________________________________________________________________________ */
void UIScrollbar::Render(void)
/*! Render the scroll bar.
*/
{
  // Render the end thumbs.
  mThumbUp->DrawRect(mX,mY,kUIElem_ScrollThumbW,kUIElem_ScrollThumbH);
  mThumbDn->DrawRect(mX,mY + (mH - kUIElem_ScrollThumbH),kUIElem_ScrollThumbW,kUIElem_ScrollThumbH);
  
  // Render the draggable thumb.
  mThumb->DrawRect(mX,
                   static_cast< int >(mY + kUIElem_ScrollThumbH + (mPosition * (mH - 3*kUIElem_ScrollThumbH))),
                   kUIElem_ScrollThumbW,
                   kUIElem_ScrollThumbH);
}


