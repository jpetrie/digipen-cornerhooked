/*! ========================================================================

      @file    asserter.h
      @author  jmp
      @brief   Interface to nsl::asserter.
      
      (c) 2004 Scientific Ninja Studios
      
    ======================================================================== */
    
/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _NSL_ASSERTER_H_
#define _NSL_ASSERTER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <windows.h>

#include <sstream>

namespace nsl
{

/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class asserter
/*! Condition assertion object.

    Assertion of a condition is accomplished by wrapping the condition in
    an assert() macro. The ASSERT() macro does nothing in release builds.
    
    Custom messages can be passed to the asserter via chained calls to the
    () operator. 
    
    Asserters have an interface similar to enforcers, with the difference
    that asserters do not throw exceptions (rather they abort execution or
    trigger a breakpoint) and asserters do nothing in release builds.
    
    @sa enforcer
*/
{
  public:
    // ct and dt
 asserter(bool test,const char *str,const char *f,unsigned int l);
    ~asserter(void);

    // * operator: assertion enforcement
    bool operator*(void);
    
    // () operator: message formatting
    template< typename D_ > asserter& operator()(const D_ &datum);
   
  private:
    // data members
    bool          mTest;     //!< Test condition result.
    std::string   mTestStr;  //!< Stringified test condition.
    const char   *mFile;     //!< File information.
    unsigned int  mLine;     //!< Line information.
    std::string   mMessage;  //!< User-supplied message.
};


/*                                                                prototypes
---------------------------------------------------------------------------- */

// construction
inline asserter make_asserter(bool test,const char *str,const char *f,unsigned int l);


/*                                                            implementation
---------------------------------------------------------------------------- */

#include "asserter.inl"


/*                                                                    macros
---------------------------------------------------------------------------- */

// assertion macro
#if defined(_DEBUG)
#define ASSERT(e) { if(*nsl::make_asserter((e),#e,__FILE__,__LINE__)) __asm int 3 }
#else
#define ASSERT(e) 
#endif


}       /* namespace nsl */
#endif  /* _NSL_ASSERTER_H_ */
