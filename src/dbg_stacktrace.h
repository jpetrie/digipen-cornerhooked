/*! ========================================================================
    
      @file    dbg_stacktrace.h
      @author  jmp
      @brief   Interface to stack trace tools.  
    
    ======================================================================== 
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DBG_STACKTRACE_H_
#define _DBG_STACKTRACE_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "wrapdbghelp.h"

namespace dbg
{

/*                                                                prototypes
---------------------------------------------------------------------------- */

// stack trace
void stacktrace(CONTEXT &c);


}       /* namespace dbg */
#endif  /* _DBG_STACKTRACE_H_ */