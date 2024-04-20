/*! ========================================================================

      @file    UIPowerMeter.cpp
      @author  jmp
      @brief   Implementation of UIPowerMeter.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
       
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIPowerMeter.h"


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIPowerMeter::UIPowerMeter(int x,int y)
/*! Constructor.
*/
: UIElement(x,y,20,300,"",""),mFillR(0),mPower(0.5f)
{
  mFillR = new DXRect(Game::Get()->GetWindow()->GetRenderer(),D3DCOLOR_ARGB(100,255,0,0),
                                                              D3DCOLOR_ARGB(100,255,0,0),
                                                              D3DCOLOR_ARGB(100,255,0,0),
                                                              D3DCOLOR_ARGB(100,255,0,0));
}

/*  ________________________________________________________________________ */
UIPowerMeter::~UIPowerMeter(void)
/*! Destructor.
*/
{
  SAFE_DELETE(mFillR);
}

/*  ________________________________________________________________________ */
void UIPowerMeter::Render(void)
/*! Render the power meter.
*/
{
std::stringstream  fmt;
int                split    = mH - static_cast< int >(mPower * mH);
int                invsplit = static_cast< int >(mPower * mH);
  fmt << mPower;

  // We have two rectangles. The first is the "empty" part, from the top
  // of the meter to the filled part.
  mRectR->DrawRect(mX,mY,mW,split);
  
  // The second is the filled part of the meter.
  mFillR->DrawRect(mX,mY + split,mW,invsplit);
  
  // Render the text.
  //mFontR->DrawText(mX + mW,mY,100,100,0xFFFFFFFF,0,fmt.str().c_str());
}