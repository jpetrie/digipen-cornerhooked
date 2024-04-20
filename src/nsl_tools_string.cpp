/*! =========================================================================

      @file    nsl_tools_string.cpp
      @brief   String tools implementation.

      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
    ========================================================================= */
 
/*                                                                   includes
----------------------------------------------------------------------------- */

#include "main.h"

#include "nsl_common.h"
#include "nsl_tools_string.h"


/*                                                                  functions
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
nsl::stringlist nsl::stsplit_chr(const std::string &str,const std::string &tok)
/*! @brief Split a string by delimiter characters.

    stsplit_chr() returns a list of substrings formed by splitting str on
    boundaries formed by any one of the characters in tok.

    @param str  The string to split.
    @param tok  The token characters used to delimit the substrings.

    @return
    An nsl::stringlist containing each of the split strings.
*/
{
nsl::stringlist  tResults;
unsigned int     tPos = 0;
unsigned int     tStart = 0;

  while(tPos < str.size())
  {
    // Find the first non-delimiter.
    while(tPos < str.size() && std::string::npos != tok.find(str[tPos]))
      ++tPos;
    
    // Find the next non-delimiter and we'll have found
    // a complete substring.
    tStart = tPos;
    while(tPos < str.size() && std::string::npos == tok.find(str[tPos]))
      ++tPos;
    
    // Only insert a legal range (e.g., a non-empty string).
    if(tPos - tStart > 0)
      tResults.push_back(str.substr(tStart,(tPos - tStart)));
  }

  return (tResults);
}



/*  _________________________________________________________________________ */
std::string nsl::stformat(const char *fmt,...)
/*! @brief Format a string.

    stformat() formats a string exactly like sprintf(), except that it
    returns a the formatted std::string.

    @param fmt  The format string.
    @param ...  Additional arguments inserted as per the format string.
    
    @return
    The formatted string.
*/
{
char     tBuffer[1024];
va_list  tArgs;

  // Idiot-proofing.
  if(0 == fmt)
    return ("");

  va_start(tArgs,fmt);
  std::vsprintf(tBuffer,fmt,tArgs);
  va_end(tArgs);
  return (std::string(tBuffer));
}


/*  _________________________________________________________________________ */
std::string nsl::stptail(const std::string &str,const std::string &sep)
/*! @brief Get the tail component of a path.

    @param str  Path string to find the tail of.
    @param str  String that delimits path components.
    
    @return
    The tail component of the path as delimited by sep.
*/
{
std::size_t  tOffset = str.find_last_of(sep);

  // If no path chars found, return the string.
  if(tOffset == std::string::npos)
    return (str);
  
  // Otherwise return the substring.
  return (str.substr(tOffset + sep.size()));
}



/*  _________________________________________________________________________ */
void nsl::stpslash(std::string &str)
/*! @brief Append a trailing backslash to the string if one does not exist.

    @param str  String to modify.
*/
{
char tTail = *(str.rbegin());

  if(('\\' != tTail) && ('/' != tTail))
  	str += '/';
}


/*  _________________________________________________________________________ */
void nsl::stpunslash(std::string &str)
/*! @brief Remove all trailing backslashes in the string if any exist.

    @param str  String to modify.
*/
{
  if(!str.empty())
  {
  const char *tBuf = str.c_str();
  size_t      tPos = size_t(str.length() - 1);

    while('\\' == tBuf[tPos] || '/' == tBuf[tPos])
      str.erase(tPos--);
  }
}