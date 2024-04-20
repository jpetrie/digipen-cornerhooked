/*! ========================================================================

      @file    enforcer.h
      @author  jmp
      @brief   Interface to nsl::enforcer.
      
      (c) 2004 Scientific Ninja Studios
      
    ======================================================================== */
    
/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _NSL_ENFORCER_H_
#define _NSL_ENFORCER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include <sstream>

namespace nsl
{


/*                                                                   structs
---------------------------------------------------------------------------- */

struct enforcer_def_predicate
/*! Default enforcement predicate.
*/
{
  template< typename T_ > 
  static bool wrong(const T_ &test);
};

struct enforcer_def_raiser
/*! Default enforcement raiser.
*/
{
  template< typename T_ > 
  static void raise(const T_ &,const std::string &message,const char *f,unsigned int l);
};


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
template< typename T_,typename P_,typename R_ >
class enforcer
/*! Condition enforcement object.

    Enforcement of a condition is accomplished by wrapping the condition in
    an ENFORCE() macro, which can be called in-line (as it returns the test).
    
    Custom messages can be passed to the enforcer via chained calls to the
    () operator. The extra message data is not processed unless the
    enforcement fails and the message needs to be displayed.
    
    Enforcers are a concept presented by Andrei Alexandrescu and Petru
    Marginean in the following article:
    
    http://www.cuj.com/documents/s=8250/cujcexp2106alexandr/alexandr.htm

    The NSL implementation is based on the code provided in the CUJ article,
    with some modifications and enhancements.

    @param T_  The test.
    @param P_  The predicate policy object.
    @param R_  The exception raiser policy object.
    
    @sa ASSERT(er
*/
{
  public:
    // ct and dt
    enforcer(T_ ref,const char *file,unsigned int line);
    ~enforcer(void);
  
    // * operator: dereference and enforcement
    T_ operator*(void);
    
    // () operator: message formatting
    template< class D_ > enforcer& operator()(const D_ &datum);
    
  private:
    // disabled
    enforcer operator=(const enforcer &s);
    
    // data members
    T_            mTest;     //!< Test condition result.
    const char   *mFile;     //!< File information.
    unsigned int  mLine;     //!< Line information.
    std::string   mMessage;  //!< User-supplied message.
};


/*                                                                prototypes
---------------------------------------------------------------------------- */

// construction
template< typename P_,typename R_,typename T_ >
inline enforcer< T_&,P_,R_ > make_enforcer(T_ &ref,const char *f,unsigned int l);
template< typename P_,typename R_,typename T_ >
inline enforcer< const T_&,P_,R_ > make_enforcer(const T_ &ref,const char *f,unsigned int l);


/*                                                            implementation
---------------------------------------------------------------------------- */

#include "enforcer.inl"


/*                                                                    macros
---------------------------------------------------------------------------- */

// enforcement macro
#define ENFORCE(e) *nsl::make_enforcer< nsl::enforcer_def_predicate,nsl::enforcer_def_raiser >((e),__FILE__,__LINE__)

}       /* namespace nsl */
#endif  /* _NSL_ENFORCER_H_ */