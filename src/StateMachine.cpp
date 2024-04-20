/*! ========================================================================

      @file    StateMachine.cpp
      @author  jmp
      @brief   Implementation of finite state machine class.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "StateMachine.h"


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
StateMachine::StateMachine(void)
/*! Default constructor.
*/
: mCurID(0),mCallEnter(0),mCallExit(0)
{
State  zeroState = { 0,0,0 };

  mStates.push_back(zeroState);
  mCurState = zeroState;
}

/*  ________________________________________________________________________ */
StateMachine::~StateMachine(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
void StateMachine::Update(float elapsed)
/*! Call the update function of the current state.
*/
{
  // Do we need to call an enter function?
  if(0 != mCallEnter)
  {
    mCallEnter(this,0.0f);
    mCallEnter = 0;
  }

  if(0 != mCurState.updateFunc) 
    mCurState.updateFunc(this,elapsed);
  
  // Do we need to call an exit function?
  if(0 != mCallExit)
  {
    mCallExit(this,0.0f);
    mCallExit = 0;
  }
}


/*  ________________________________________________________________________ */
bool StateMachine::TransitionTo(unsigned int stateID)
/*! Transition to a specified state.

    @param stateID  The state ID to transition to.
    
    @return
    True if the transition succeded, false otherwise.
    If true is returned, the current state's exit function will be called
    after the end of the currently executing update (if TransitionTo() was
    called within an update). Prior to the beginning of the next update, the
    new states enter function will be called.
*/
{
  // Check for bad state.
  if(stateID > mStates.size() || stateID == 0)
    return (false);

  mCallExit  = mCurState.exitFunc;
  mCurState  = mStates[stateID - 1];
  mCurID     = stateID - 1;
  mCallEnter = mCurState.enterFunc;

  return (true);
}

/*  ________________________________________________________________________ */
unsigned int StateMachine::DefineState(StateMachine::TStateFunction update,StateMachine::TStateFunction enter,StateMachine::TStateFunction exit)
/*! Define a state for the state machine.

    @param update  The update function for this state.
    @param enter   The enter function for this state (may be null).
    @param exit    The exit function for this state (may be null).
    
    @return
    An integer that is the unique ID for the state (if zero, state definition failed).
    This ID is needed to transition between states.
*/
{
unsigned int id = static_cast< unsigned int >(mStates.size() + 1);
State        s;

  if(0 == update)
    return (0);

  s.updateFunc = update;
  s.enterFunc  = enter;
  s.exitFunc   = exit;

  mStates.push_back(s);
  return (id);
}