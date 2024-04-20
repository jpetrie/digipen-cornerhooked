/*! ========================================================================

      @file    UIButton.cpp
      @author  jmp
      @brief   Implementation of UIButton.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIButton.h"

#include "Skybox.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIButton::UIButton(int x,int y,int w,int h,const std::string &caption,int textSize)
/*! Constructor.
*/
: UIElement(x,y,w,h,"",caption,textSize)
{
}

/*  ________________________________________________________________________ */
UIButton::~UIButton(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
void UIButton::Render(void)
/*! Render the element.
*/
{
DWORD  color = mIsEnabled ? Skybox::UIText() : Skybox::UIDisableText();

  // Render the background, render the text.
  mRectR->DrawRect(mX,mY,mW,mH);
  mFontR->DrawText(mX,mY,mW,mH,color,DT_CENTER | DT_VCENTER,mCaption);
}