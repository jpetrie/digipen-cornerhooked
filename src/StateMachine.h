/*! ========================================================================

      @file    StateMachine.h
      @author  jmp
      @brief   Interface to finite state machine class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <stack>
#include <vector>


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class StateMachine
/*! Finite state machine class.
*/
{
  public:
    // typedefs
    typedef void (*TStateFunction)(StateMachine*,float);
    
    // ct and dt
    StateMachine(void);
    ~StateMachine(void);
    
    // update
    void Update(float elapsed);
    
    // transition
    bool TransitionTo(unsigned int stateID);
    
    // states
    unsigned int DefineState(TStateFunction update,TStateFunction enter,TStateFunction exit);
    unsigned int CurrentState(void) const { return (mCurID + 1); }
  
  private:
    // structs
    struct State
    {
      TStateFunction  updateFunc;
      TStateFunction  enterFunc;
      TStateFunction  exitFunc;
    };
  
    // data members
    State           mCurState;
    unsigned int    mCurID;
    TStateFunction  mCallEnter;
    TStateFunction  mCallExit;
    
    std::vector< State >  mStates;
};


#endif  /* _STATEMACHINE_H_ */