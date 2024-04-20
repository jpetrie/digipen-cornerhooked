/*! ========================================================================

      @file    UIGraphic.cpp
      @author  jmp
      @brief   Implementation of UIGraphic.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================  */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIGraphic.h"



/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
UIGraphic::UIGraphic(int x,int y,int w,int h,const std::string &texture)
/*! Constructor.
*/
: UIElement(x,y,w,h,texture,"")
{
  // Graphic elements should be disabled by default.
  Enable(false);
}

/*  ________________________________________________________________________ */
UIGraphic::~UIGraphic(void)
/*! Destructor.
*/
{
}