/* =========================================================================
   
    @file    Clock.cpp
    @author  jmp
    @brief   Implementation of clock class.
    
   ========================================================================= */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Clock.h"


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Clock::Clock(void)
/*! Default constructor.
*/
: mFirst(true),mElapsed(0.0f),mAccumulated(0.0f),mLimit(0.1f)
{
  // Get the timer frequency.
  ::QueryPerformanceFrequency(&mClockFreq);
  
  // Zero counts, initially.
  mStartTime.QuadPart = 0;
  mEndTime.QuadPart = 0;
}

/*  ________________________________________________________________________ */
Clock::~Clock(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
void Clock::Update(void)
/*! Begin a frame period.
*/
{
  ::QueryPerformanceCounter(&mStartTime);
  
  // Calculate elapsed time, in seconds.
  // The first time, the elapsed time is zero (otherwise we'll calculate a
  // massive timestep).
  if(mFirst)
  {
    mElapsed = 0.0f;
    mFirst   = false;
  }
  else
    mElapsed = (float)(mStartTime.QuadPart - mEndTime.QuadPart) / (float)(mClockFreq.QuadPart);
    
  // Clamp if over limit.
  if(mElapsed >= mLimit)
    mElapsed = mLimit;
  mAccumulated += mElapsed;
  
  mEndTime = mStartTime;
}

/*  ________________________________________________________________________ */
void Clock::Reset(void)
/*! Reset accumulated time.
*/
{
  mAccumulated = 0.0f;
}

/*  ________________________________________________________________________ */
float Clock::Elapsed(void)
/*! Return the elapsed time since last frame.
*/
{
  return (mElapsed);
}

/*  ________________________________________________________________________ */
float Clock::Accumulated(void)
/*! Return the accumulated time since clock start.
*/
{
///@bug This can technically hit a ceiling eventually....
  return (mAccumulated);
}