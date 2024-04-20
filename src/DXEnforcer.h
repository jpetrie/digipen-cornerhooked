/*! ========================================================================

      @file    DXEnforcer.h
      @author  jmp
      @brief   Interface to DirectX enforcer.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_ENFORCER_H_
#define _DX_ENFORCER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "enforcer.h"


/*                                                                   structs
---------------------------------------------------------------------------- */

struct DXEnforcerPredicate
//! DirectX enforcement predicate.
{
  template< typename T_ > 
  static bool wrong(const T_ &test);
};

struct DXEnforcerRaiser
//! DirectX enforcement raiser.
{
  template< typename T_ > 
  static void raise(const T_ &test,const std::string &message,const char *f,unsigned int l);
};


/*                                                            implementation
---------------------------------------------------------------------------- */

#include "DXEnforcer.inl"


/*                                                                    macros
---------------------------------------------------------------------------- */

// enforcement macro
#define ENFORCE_DX(e) *nsl::make_enforcer< DXEnforcerPredicate,DXEnforcerRaiser >((e),__FILE__,__LINE__)


#endif  /* _DX_ENFORCER_H_ */