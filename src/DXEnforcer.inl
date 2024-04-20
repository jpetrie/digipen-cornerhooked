/*! ========================================================================

      @file    DXEnforcer.inl
      @author  jmp
      @brief   Implementation of DirectX enforcer.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
template< typename T_ > 
bool DXEnforcerPredicate::wrong(const T_ &test)
/* Test a reference for "wrongess."

   @param test  The reference to test.
   
   @return
   True if the reference is "wrong," false otherwise.
*/
{
  // Return true if "wrong", e.g.:
  return (FAILED(test));
}

/*  ________________________________________________________________________ */
template< typename T_ > 
void DXEnforcerRaiser::raise(const T_ &test,const std::string &message,const char *f,unsigned int l)
/*! Raise an exception.

    @param test      The test value.
    @param message   Message.
    @param f         File information.
    @param l         Line information.
*/
{
std::ostringstream  fmt;

  // Format and throw the message.
  fmt << message << "\n(" << f << ":" << l << "; " << DXGetErrorString9(test) << ")";
  throw (std::runtime_error(fmt.str().c_str()));
}