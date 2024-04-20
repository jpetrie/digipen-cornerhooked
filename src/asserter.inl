/*! ========================================================================

      @file    asserter.inl
      @author  jmp
      @brief   Implementation of nsl::asserter.
      
      (c) 2004 Scientific Ninja Studios
      
    ======================================================================== */
    
/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
template< typename D_ >
asserter& asserter::operator()(const D_ &datum)
/*! Append message data.

    Anything that can be streamed to a standard output stream can be appended
    to an asserter using this operator.
    
    @return
    A reference to the asserter, in order to support call chaining.
*/
{
std::ostringstream  fmt;
    
  // Append to stored message.
  fmt << datum;
  mMessage += fmt.str();
  
  // Return *this so we can chain calls to this operator.
  return (*this);
}

/*  ________________________________________________________________________ */
asserter make_asserter(bool test,const char *str,const char *f,unsigned int l)
/*! Construct an assertion from a condition.

    @param test  The condition to test.
    @param str   The stringified condition.
    @param f     The file where the assertion is occuring.
    @param l     The line where the assertion is occuring.

    @return
    An asserter object for the given condition.
*/
{
  return (asserter(test,str,f,l));
}