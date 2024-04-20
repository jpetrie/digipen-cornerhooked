/*! =========================================================================
      
      @file    lexical_cast.h
      @author  jmp
      @brief   lexical_cast interface.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
       
    ========================================================================= */

/*                                                                      guard
----------------------------------------------------------------------------- */

#ifndef _LEXICALCAST_H_
#define _LEXICALCAST_H_


/*                                                                    classes
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
template< typename R_,typename T_ > R_ lexical_cast(T_ src)
/*!
*/
{
std::stringstream  fmt;
R_                 result;

  fmt << src;
  fmt >> result;
  return (result);
}

#endif  /* _LEXICALCAST_H_ */
