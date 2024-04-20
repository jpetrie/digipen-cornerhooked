/*! =========================================================================
      
      @file    NLBytestream.h
      @brief   nsl::bstream interface.
      
    ========================================================================= 
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */
    
/*                                                                      guard
----------------------------------------------------------------------------- */
                                                                  
#ifndef _NSL_BSTREAM_H_
#define _NSL_BSTREAM_H_


/*                                                                   includes
----------------------------------------------------------------------------- */

#include "nsl_common.h"

namespace nsl
{


/*                                                                    classes
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
class bstream
/*! @brief Provides a streaming interface to an array of bytes.

    The bytestream provides methods for inserting and extracting most primitive
    data types using the stream semantics, as well as assigning and mapping a
    block of memory to the stream.

    The bytestream stores its data internally in network byte order; conversion
    is performed during insertion and extraction as appropriate, however, it
    is not possible for any such conversion to be performed when assigning or
    mapping blocks of memory, so care must be used when calling those methods.
    
    A bytestream may be safely copied.
*/
{
  public:
    // ct and dt
     bstream(void);
     bstream(const bstream &s);
    ~bstream(void);
  
    // = operator: assignment
    bstream& operator=(const bstream &rhs);
  
    // accessors
    const byte_t* data(void) const     { return (mData); }
          size_t  size(void) const     { return (scast< size_t >(mPutPtr - mGetPtr)); }
          size_t  capacity(void) const { return (mCapacity); }

    // manipulators
    void erase(void);
          
    // << operators
    bstream& operator<<(const std::string &data);
    bstream& operator<<(int data);
    bstream& operator<<(unsigned int data);
    bstream& operator<<(char data);
    bstream& operator<<(unsigned char data);
    bstream& operator<<(short data);
    bstream& operator<<(unsigned short data);
    bstream& operator<<(long data);
    bstream& operator<<(unsigned long data);
    bstream& operator<<(float data);
    bstream& operator<<(double data);
    
    // >> operators
    bstream& operator>>(std::string &data);
    bstream& operator>>(int &data);
    bstream& operator>>(unsigned int &data);
    bstream& operator>>(char &data);
    bstream& operator>>(unsigned char &data);
    bstream& operator>>(short &data);
    bstream& operator>>(unsigned short &data);
    bstream& operator>>(long &data);
    bstream& operator>>(unsigned long &data);
    bstream& operator>>(float &data);
    bstream& operator>>(double &data);
    
    // low-level
    byte_t* raw_get(void);
    void    raw_set(const byte_t *src,size_t cap);
    void    raw_put(const byte_t *src,size_t sz);
    
    
  private:
    // sizing
    void nExpand(void);
    
    // data members
    byte_t *mData;    // Byte buffer.
    byte_t *mPutPtr;  // Points to address of next inserted value.
    byte_t *mGetPtr;  // Points to address of next extracted value.
    
    unsigned long  mCapacity;  // Maximum size of current buffer.
};

}       /* namespace nsl */
#endif  /* _NSL_BSTREAM_H_ */