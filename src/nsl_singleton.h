/*! =========================================================================
      
      @file    nsl_singleton.h
      @brief   nsl::singleton interface.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
       
    ========================================================================= */

/*                                                                      guard
----------------------------------------------------------------------------- */

#ifndef _NSL_SINGLETON_H_
#define _NSL_SINGLETON_H_


/*                                                                   includes
----------------------------------------------------------------------------- */

#include "nsl_common.h"

namespace nsl
{


/*                                                                    classes
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
template < typename T_ >
class singleton
/*! @brief Singleton base class.

    @param T_  The mix-in target.
*/
{
  // sterotypes
  nlCONTRACT_NOCOPY(singleton);

  public:
    // ct and dt
             singleton(void);
    virtual ~singleton(void);
    
    // instance accessors
    static bool  exists(void) { return (0 != mInstance); }
    static T_*   instance(void);
    static T_*   Get(void)    { return (instance()); }
  
  private:
    // data members
    static T_* mInstance;
};


/*                                                             static members
----------------------------------------------------------------------------- */

// singleton instance
template< typename T_ > T_* singleton< T_ >::mInstance = 0;


/*                                                           function members
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
template< typename T_ >
singleton< T_ >::singleton(void)
/*! @brief Default constructor.
*/
{
  // Halt on multiple-creation.
  ASSERT(0 == mInstance);
  
  // Cast the instance pointer to the base type.
  mInstance = scast< T_* >(this);
}


/*  _________________________________________________________________________ */
template< typename T_ >
singleton< T_ >::~singleton(void)
/*! @brief Destructor.
*/
{
  mInstance = 0;
}


/*  _________________________________________________________________________ */
template< typename T_ >
T_* singleton< T_ >::instance(void)
/*! @brief Recover the singleton instance.

    @return
    A mutable pointer to the singleton instance.
*/
{
  ASSERT(0 != mInstance);
  return (mInstance);
}


}       /* namespace nsl */
#endif  /* _NSL_SINGLETON_H_ */
