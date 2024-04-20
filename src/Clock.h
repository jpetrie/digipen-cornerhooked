/* =========================================================================
   
    @file    Clock.h
    @author  jmp
    @brief   Interface to clock class.
    
   ========================================================================= */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _CLOCK_H_
#define _CLOCK_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <windows.h>


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Clock
/*! A clock. 
*/
{ 
  public:
    // ct and dt
    Clock(void);
    ~Clock(void);
  
    // update
    void Update(void);
    void Reset(void);
    
    // elapsed
    float Elapsed(void);
    float Accumulated(void);
  
  private:
    // data members
    bool           mFirst;
    
    float          mLimit;  //!< Elapsed time is not allowed to exceed this value.
    
    float          mElapsed;
    float          mAccumulated;
    
    LARGE_INTEGER  mStartTime;
    LARGE_INTEGER  mEndTime;
    LARGE_INTEGER  mClockFreq;  //!< For changing timer units into seconds.
};

#endif  /* _CLOCK_H_ */