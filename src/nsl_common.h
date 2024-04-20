/*! =========================================================================
      
      @file    nsl_common.h
      @brief   NSL common header.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================= */

/*                                                                      guard
----------------------------------------------------------------------------- */

#ifndef _NSL_COMMON_
#define _NSL_COMMON_


/*                                                              configuration
----------------------------------------------------------------------------- */

#if defined(_MSC_VER)
  #define NSL_VC
#elif defined(__BORLANDC__)
  #define NSL_BCB
#else
  #error "unknown compiler"
#endif


/*                                                                   includes
----------------------------------------------------------------------------- */

#include <windows.h>

#include <cmath>
#include <cstdarg>

#pragma warning( push, 3 )
#pragma warning( disable: 4702 )
#include <vector>
#include <fstream>
#include <string>
#pragma warning( pop )


/*                                                                   typedefs
----------------------------------------------------------------------------- */

namespace nsl
{
  // integer types
  typedef signed int      int_t;
  typedef signed int      sint_t;
  typedef unsigned int    uint_t;
  typedef signed char     int8_t;
  typedef signed char     sint8_t;
  typedef unsigned char   uint8_t;
  typedef signed short    int16_t;
  typedef signed short    sint16_t;
  typedef unsigned short  uint16_t;
  typedef signed long     int32_t;
  typedef signed long     sint32_t;
  typedef unsigned long   uint32_t;

  // real types
  typedef float   float_t;
  typedef double  double_t;

  // high level types
  typedef uint8_t   byte_t;
  typedef uint32_t  size_t;
  typedef uint32_t  index_t;
  typedef sint32_t  offset_t;
  
  // character types
  typedef byte_t    char_t;
  
  // boolean types
  typedef bool  bool_t;
}


/*                                                                     macros
----------------------------------------------------------------------------- */


// C++ cast sugar
#define scast  static_cast
#define dcast  dynamic_cast
#define rcast  reinterpret_cast
#define ccast  const_cast

// multichar constant macros
#define nlFOURCODE(a,b,c,d)  (((a) << 24) | ((b) << 16 ) | ((c) << 8) | d)

// contract macros
#define nlCONTRACT_NOCOPY(c)    private: c(const c &); const c& operator=(const c &s)   


#endif  /* _NSL_COMMON_ */
