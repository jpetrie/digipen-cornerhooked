/*! ========================================================================

      @file    UIPowerMeter.h
      @author  jmp
      @brief   Interface to UIPowerMeter class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_POWERMETER_H_
#define _UI_POWERMETER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "UIElement.h"

#include "Game.h"

/*                                                                 constants
---------------------------------------------------------------------------- */

// colors
const DWORD kUIElem_PMeterFillColor  = D3DCOLOR_ARGB(100,0,255,255);


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIPowerMeter : public UIElement
/*! A shot power meter.

    The power meter stores the power of the shot as a float between 0 and 1.
*/
{
  public:
    // ct and dt
    UIPowerMeter(int x,int y);
    ~UIPowerMeter(void);
    
    // accessors
    virtual Type GetType(void) const { return (kPowerMeter); }
  
    // power access
    float GetPower(void) const { return (mPower); }
    void  SetPower(float p)    { if(p > 1.0f) p = 1.0f; if(p < 0.0f) p = 0.0f; mPower = p; }
  
    // render
    virtual void Render(void);
    
  private:
    // data members
    DXRect *mFillR;  //!< Rectangle render for filled portion of meter.
    
    float   mPower;  //!< Power stored in the meter, [0..1]. 
};


#endif  /* _UI_POWERMETER_H_ */