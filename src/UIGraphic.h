/*! ========================================================================

      @file    UIGraphic.h
      @author  jmp
      @brief   Interface to UIGraphic class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_GRAPHIC_H_
#define _UI_GRAPHIC_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "UIElement.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIGraphic : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIGraphic(int x,int y,int w,int h,const std::string &texture);
    ~UIGraphic(void);
    
    // accessors
    virtual Type GetType(void) const { return (kGraphic); }
};


#endif  /* _UI_GRAPHIC_H_ */