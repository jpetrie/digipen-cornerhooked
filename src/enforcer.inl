/*! ========================================================================

      @file    enforcer.inl
      @author  jmp
      @brief   Implementation of nsl::enforcer.
      
      (c) 2004 Scientific Ninja Studios
      
    ======================================================================== */
    
/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
template< typename T_ > 
bool enforcer_def_predicate::wrong(const T_ &test)
/* Test for "wrongness."

   @param test  The reference to test.
   
   @return
   True if the test is "wrong," false otherwise.
*/
{
  // Return true if "wrong", e.g.:
  return (!test);
}

/*  ________________________________________________________________________ */
template< typename T_ > 
void enforcer_def_raiser::raise(const T_ &/*test*/,const std::string &message,const char *f,unsigned int l)
/*! Raise an exception.

    @param test      The test.
    @param message   The message.
    @param f         File information.
    @param l         Line information.
*/
{
std::ostringstream  fmt;

  // Format and throw the message.
  fmt << message << "\n(" << f << ":" << l << ")";
  throw (std::runtime_error(fmt.str().c_str()));
}

/*  ________________________________________________________________________ */
template< typename T_,typename P_,typename R_ >
enforcer< T_,P_,R_ >::enforcer(T_ ref,const char *file,unsigned int line)
/*! Constructor.
    
    The constructor will leave the enforcer partially-constructed (no file
    information) if the enforcement succeded. Other methods test for this
    partially-constructed state to determine if they should do any work.
    
    @param ref   The condition to enforce.
    @param file  The file where the enforcement is occuring.
    @param line  The line where the enforcement is occuring.
*/
: mTest(ref),mFile(P_::wrong(ref) ? file : 0),mLine(line)
{
}

/*  ________________________________________________________________________ */
template< typename T_,typename P_,typename R_ >
enforcer< T_,P_,R_ >::~enforcer(void)
/*! Destructor.
*/
{
}

/*  ________________________________________________________________________ */
template< typename T_,typename P_,typename R_ >
T_ enforcer< T_,P_,R_ >::operator*(void)
/*! Test the enforcement and raise an exception.
    
    The raise policy type's raise() method is invoked if the enforcer is
    fully-constructed (e.g., enforcement failed). Otherwise, the test 
    reference is returned to facilitate calling ENFORCE() in-line.
    
    @return
    The test.
*/
{
  if(0 != mFile)
  {
  const char *ptr  = mFile;
  const char *file = mFile;
  
    // Take just the tail of the file string.
    while(*ptr != 0)
    {
      if(*ptr == '\\' || *ptr == '/')
        file = ++ptr;   
      else
        ++ptr;
    }
    
    R_::raise(mTest,mMessage,file,mLine);
  }
  return (mTest);
}

/*  ________________________________________________________________________ */
template< typename T_,typename P_,typename R_ >
template< typename D_ >
enforcer< T_,P_,R_ >& enforcer< T_,P_,R_ >::operator()(const D_ &datum)
/*! Test the enforcement and append message data.

    The message data is only appended if the enforcement will fail.
    Anything that can be streamed to a standard output stream can be appended
    to an enforcer using this operator.
    
    @return
    A reference to the enforcer, in order to support call chaining.
*/
{
  if(mFile) 
  {
  std::ostringstream  fmt;
    
    // Append to stored message.
    fmt << datum;
    mMessage += fmt.str();
  }
  
  // Return *this so we can chain calls to this operator.
  return (*this);
}


/*  ________________________________________________________________________ */
template< typename P_,typename R_,typename T_ >
inline enforcer< T_&,P_,R_ > make_enforcer(T_ &ref,const char *f,unsigned int l)
/*! Construct an enforce from a condition.
    
    @param ref  The condition reference.
    @param f    File where the enforcement is occuring.
    @param l    Line where the enforcement is occuring.
    
    @return
    The enforcer for the condition.
*/
{
  return (enforcer< T_&,P_,R_ >(ref,f,l));
}

/*  ________________________________________________________________________ */
template< typename P_,typename R_,typename T_ >
inline enforcer< const T_&,P_,R_ > make_enforcer(const T_ &ref,const char *f,unsigned int l)
/*! Construct an enforce from a constant condition.
    
    @param ref  The condition reference.
    @param f    File where the enforcement is occuring.
    @param l    Line where the enforcement is occuring.
    
    @return
    The enforcer for the condition.
*/
{
  return (enforcer< const T_&,P_,R_ >(ref,f,l));
}