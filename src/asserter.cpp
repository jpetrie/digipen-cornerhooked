/*! ========================================================================

      @file asserter.cpp
      @author  jmp
      @brief   Implementation of nsl::asserter.
      
      (c) 2004 Scientific Ninja Studios
      
    ======================================================================== */
    
/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "asserter.h"

namespace nsl
{


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
asserter::asserter(bool test,const char *str,const char *f,unsigned int l)
/*! Constructor.

    @param test  The condition to test.
    @param str   The stringified condition.
    @param f     The file where the assertion is occuring.
    @param l     The line where the assertion is occuring.
*/
: mTest(test),mTestStr(str),mFile(f),mLine(l)
{
}

/*  ________________________________________________________________________ */
asserter::~asserter(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
bool asserter::operator*(void)
/*! Test the assertion.

    If the assertion failed, this function informs the user.
    
    @return
    True if the assertion failed, false otherwise.
*/
{
  if(!mTest)
  {
  std::ostringstream  fmt;
  const char         *ptr  = mFile;
  const char         *file = mFile;
  
    // Take just the tail of the file string.
    while(*ptr != 0)
    {
      if(*ptr == '\\' || *ptr == '/')
        file = ++ptr;   
      else
        ++ptr;
    }
    
    // Format the message.
    fmt << "(" << mTestStr << ") at " << file << ":" << mLine;
    if(!mMessage.empty())
      fmt << "\n\n" << mMessage;
    ::MessageBox(0,fmt.str().c_str(),"assertion Failed",MB_OK);
  }
  
  return (!mTest);
}

}  /* namespace nsl */