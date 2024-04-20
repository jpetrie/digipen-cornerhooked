/*! ========================================================================

      @file    UIScrollbar.h
      @author  jmp
      @brief   Interface to UIScrollbar class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _UI_SCROLLBAR_H_
#define _UI_SCROLLBAR_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "UIElement.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

const int  kUIElem_ScrollbarW      = 16;
const int  kUIElem_ScrollThumbW    = 16;
const int  kUIElem_ScrollThumbH    = 16;
const int  kUIElem_ScrollbarOffset = 2;


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class UIScrollbar : public UIElement
/*! 
*/
{
  public:
    // ct and dt
    UIScrollbar(int x,int y,int w,int h,float pos = 0.0f);
    ~UIScrollbar(void);
    
    // accessors
    virtual Type GetType(void) const { return (kScrollbar); }
    
    // min/max/cur
    int  GetMin(void) const { return (mMin); }
    int  GetMax(void) const { return (mMax); }
    int  GetCur(void) const { return (mCur); }
    void SetMin(int v)      { mMin = v; }
    void SetMax(int v)      { mMax = v; }
    void SetCur(int v)      { mCur = v; }
    
    // step
    int  GetStep(void) const { return (mStep); }
    void SetStep(int s)      { mStep = s; }
    void StepUp(unsigned int step = 1);
    void StepDn(unsigned int step = 1);
    
    // render
    virtual void Render(void);
    
    // events
    virtual bool CheckLeftClick(int x,int y);
  
  private:
    // data members
    float  mPosition;
    
    int  mMin;   //!< Min value.
    int  mMax;   //!< Max value.
    int  mCur;   //!< Current value.
    int  mStep;  //!< Amount to step each time the endpoints are clicked.
    
    DXRect  *mThumb;
    DXRect  *mThumbUp;
    DXRect  *mThumbDn;
};


#endif  /* _UI_SCROLLBAR_H_ */