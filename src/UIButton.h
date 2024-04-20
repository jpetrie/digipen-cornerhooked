/*! ========================================================================

      @file    UIButton.h
      @author  jmp
      @brief   Interface to UIButton class.
      
    ======================================================================== 
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_BUTTON_H_
#define _UI_BUTTON_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "UIElement.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIButton : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIButton(int x,int y,int w,int h,const std::string &caption,int textSize = kUIElem_DefaultTextSize);
    ~UIButton(void);
    
    // accessors
    virtual Type GetType(void) const { return (kPushButton); }
    
    // render
    virtual void Render(void);
};


#endif  /* _UI_BUTTON_H_ */