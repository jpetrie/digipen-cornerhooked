/*! =========================================================================
      
      @file    nsl_tools_string.h
      @brief   String tools interface.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ========================================================================= */

/*                                                                      guard
----------------------------------------------------------------------------- */

#ifndef _NSL_TOOLS_STRING_H_
#define _NSL_TOOLS_STRING_H_


/*                                                                   includes
----------------------------------------------------------------------------- */
              
#include "main.h"
  
#include "nsl_common.h"

namespace nsl
{


/*                                                                   typedefs
----------------------------------------------------------------------------- */

typedef std::vector< std::string >  stringlist;


/*                                                                 prototypes
----------------------------------------------------------------------------- */

// tokenization
stringlist stsplit_chr(const std::string &str,const std::string &tok);

// format
std::string stformat(const char *fmt,...);

// path strings
std::string stptail(const std::string &str,const std::string &sep = "/");
void        stpslash(std::string &str);
void        stpunslash(std::string &str);


}       /* namespace nsl */
#endif  /* _NSL_TOOLS_STRING_H_ */