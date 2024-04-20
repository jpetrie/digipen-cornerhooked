/*! =========================================================================
 
     @file    nsl_bstream.cpp
     @brief   nsl::bstream implementation.
    
     (c) 2004 DigiPen (USA) Corporation, all rights reserved.
     
    ========================================================================= */
 
/*                                                                   includes
----------------------------------------------------------------------------- */

#include "main.h"
                                                                    
#include "nsl_common.h"
#include "nsl_bstream.h"

                                                                    
/*                                                         internal constants 
----------------------------------------------------------------------------- */


// defaults
namespace
{
  const nsl::size_t  ikDefaultCapacity     = 256;
  const nsl::size_t  ikDefaultResizeFactor = 2;
}


/*                                                           member functions
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
nsl::bstream::bstream(void)
/*! @brief Default constructor.
*/
: mData(new nsl::byte_t[ikDefaultCapacity]),
  mPutPtr(mData),
  mGetPtr(mData),
  mCapacity(ikDefaultCapacity)  
{
  erase();
}


/*  _________________________________________________________________________ */
nsl::bstream::bstream(const nsl::bstream &s)
/*! @brief Copy constructor.

    Performs a deep copy of the source bytestream to the invoking bytestream.
    
    @param s  The source bytestream.
*/
: mData(new nsl::byte_t[s.size()]),
  mPutPtr(mData),
  mGetPtr(mData),
  mCapacity(s.size())  
{
  erase();
  raw_put(s.data(),s.size());
}


/*  _________________________________________________________________________ */
nsl::bstream::~bstream(void)
/*! @brief Destructor.
*/
{
  delete[] mData;
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator=(const nsl::bstream &rhs)
/*! @brief Assign a bytestream to another bytestream.

    Performs a deep copy of rhs to the invoking object, destroying the
    previous contents of the invoking object.

    @param rhs  The bytestream to copy from.
    
    @return
    A reference to the invoking object.
*/
{
  erase();
  raw_put(rhs.data(),rhs.size());
  return (*this);
}


/*  _________________________________________________________________________ */
void nsl::bstream::erase(void)
/*! @brief Erase the contents of the stream.

    erase() zeroes the entire block of raw memory used by the stream, and sets
    the stream's size to zero. It does not alter the capacity of the stream.
*/
{
  memset(mData,0,mCapacity);
  mPutPtr = mData;
  mGetPtr = mData;
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(const std::string &data)
/*! @brief Insert data into the stream.

    Strings are inserted without their terminator, but prefixed by a 16-bit
    value that specifies their length. If the string length is greater than
    the capacity of a 16-bit unsigned integer, only part of the string is
    written.

    @param data  Data to insert.
    
    @return
    A reference to the stream.
*/
{
nsl::uint16_t  tSize = scast< nsl::uint16_t >(data.length());
nsl::uint16_t  tSwappedSize = htons(tSize);

  if(size() + tSize + sizeof(tSize) > capacity())
    nExpand();

  memcpy(mPutPtr,&tSwappedSize,sizeof(tSwappedSize));
  mPutPtr += sizeof(tSwappedSize);
  memcpy(mPutPtr,data.c_str(),tSize);
  mPutPtr += tSize;
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(int data)
/*! @brief Insert data into the stream.
	
	    @param data  Data to insert.
	    
	    @return
	    A reference to the stream.
	*/
{
nsl::sint_t  tSwapped = htonl(data);

	if(size() + sizeof(tSwapped) > capacity())
	  nExpand();

	memcpy(mPutPtr,&tSwapped,sizeof(tSwapped));
	mPutPtr += sizeof(tSwapped);
	
	return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(unsigned int data)
/*! @brief Insert data into the stream.
	
	    @param data  Data to insert.
	    
	    @return
	    A reference to the stream.
	*/
{
nsl::uint_t  tSwapped = htonl(data);

	if(size() + sizeof(tSwapped) > capacity())
	  nExpand();

	memcpy(mPutPtr,&tSwapped,sizeof(tSwapped));
	mPutPtr += sizeof(tSwapped);
	
	return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(char data)
/*! @brief Insert data into the stream.

	  @param data  Data to insert.
	  
	  @return
	  A reference to the stream.
*/
{
	if(size() + sizeof(data) > capacity())
	  nExpand();

	memcpy(mPutPtr,&data,sizeof(data));
	mPutPtr += sizeof(data);
	
	return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(unsigned char data)
/*! @brief Insert data into the stream.

	  @param data  Data to insert.
	  
	  @return
	  A reference to the stream.
*/
{
	if(size() + sizeof(data) > capacity())
	  nExpand();

	memcpy(mPutPtr,&data,sizeof(data));
	mPutPtr += sizeof(data);
	
	return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(short data)
/*! @brief Insert data into the stream.

	  @param data  Data to insert.
	  
	  @return
	  A reference to the stream.
*/
{
nsl::sint16_t  tSwapped = htons(data);

	if(size() + sizeof(tSwapped) > capacity())
	  nExpand();

	memcpy(mPutPtr,&tSwapped,sizeof(tSwapped));
  mPutPtr += sizeof(tSwapped);

  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(unsigned short data)
/*! @brief Insert data into the stream.

	  @param data  Data to insert.
	  
	  @return
	  A reference to the stream.
*/
{
nsl::uint16_t  tSwapped = htons(data);

	if(size() + sizeof(tSwapped) > capacity())
	  nExpand();

	memcpy(mPutPtr,&tSwapped,sizeof(tSwapped));
  mPutPtr += sizeof(tSwapped);

  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(long data)
/*! @brief Insert data into the stream.

    @param data  Data to insert.
    
    @return
    A reference to the stream.
*/
{
nsl::sint32_t  tSwapped = htonl(data);

  if(size() + sizeof(tSwapped) > capacity())
    nExpand();

  memcpy(mPutPtr,&tSwapped,sizeof(tSwapped));
  mPutPtr += sizeof(tSwapped);
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(unsigned long data)
/*! @brief Insert data into the stream.

    @param data  Data to insert.
    
    @return
    A reference to the stream.
*/
{
nsl::uint32_t  tSwapped = htonl(data);

  if(size() + sizeof(tSwapped) > capacity())
    nExpand();

  memcpy(mPutPtr,&tSwapped,sizeof(tSwapped));
  mPutPtr += sizeof(tSwapped);
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(float data)
/*! @brief Insert data into the stream.

    @param data  Data to insert.
    
    @return
    A reference to the stream.
*/
{
  if(size() + sizeof(data) > capacity())
    nExpand();

  memcpy(mPutPtr,&data,sizeof(data));
  mPutPtr += sizeof(data);
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator<<(double data)
/*! @brief Insert data into the stream.

    @param data  Data to insert.
    
    @return
    A reference to the stream.
*/
{
  if(size() + sizeof(data) > capacity())
    nExpand();

  memcpy(mPutPtr,&data,sizeof(data));
  mPutPtr += sizeof(data);
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(std::string &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
  nsl::uint16_t  tSize;
  nsl::char_t   *tData;
 
    memcpy(&tSize,mGetPtr,sizeof(tSize));
    tSize = ntohs(tSize);
    mGetPtr += sizeof(tSize);
    tData = new nsl::char_t[tSize + 1];
    memcpy(tData,mGetPtr,tSize);
    tData[tSize] = 0;
    data = rcast< char* >(tData);
    delete[] tData;
    memset(mGetPtr,0,tSize);
    mGetPtr += tSize;
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(int &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.
    
    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
    data = ntohl(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(unsigned int &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.
    
    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
    data = ntohl(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(char &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(unsigned char &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(short &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
    data = ntohs(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(unsigned short &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
    data = ntohs(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(long &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
    data = ntohl(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(unsigned long &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
    data = ntohl(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(float &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
  }
  
  return (*this);
}


/*  _________________________________________________________________________ */
nsl::bstream& nsl::bstream::operator>>(double &data)
/*! @brief Extract data from the stream.

    If there is no data in the stream, this function does nothing and the value
    of the parameter is unchanged. It is up to the caller to ensure that the
    data in the stream matches the type of the parameter, or the value of the
    parameter will not be what was expected.

    @param data  Recieves extracted data.
    
    @return
    A reference to the stream.
*/
{
  if(mGetPtr < mPutPtr)
  {
    memcpy(&data,mGetPtr,sizeof(data));
    memset(mGetPtr,0,sizeof(data));
    mGetPtr += sizeof(data);
  }
  
  return (*this);
}  

/*  _________________________________________________________________________ */
nsl::byte_t* nsl::bstream::raw_get(void)
/*! @brief Get the stream's raw memory.

    raw_get() returns a pointer to the first byte of valid memory used by the
    stream (e.g., the get pointer).

    @return
    A pointer to the raw memory used by the stream. Unlike the pointer returned
    by data(), the return value of raw_get() is mutable.
*/
{
  return (mGetPtr);
}


/*  _________________________________________________________________________ */
void nsl::bstream::raw_set(const nsl::byte_t *src,nsl::size_t cap)
/*! @brief Set the stream's raw memory.

    raw_set() sets the stream's capacity to cap, and copies cap bytes from
    src into the stream's internal buffer. src may be null. This
    invalidates the current contents of the stream and any pointers to it.

    @param src  Source buffer, may be null.
    @param cap  New memory capacity.
*/
{
  delete[] mData;
  mData = new nsl::byte_t[cap];
  if(0 != src)
    memcpy(mData,src,cap);

  mGetPtr = mData;
  mPutPtr = mData + cap;
  mCapacity = cap;
}


/*  _________________________________________________________________________ */
void nsl::bstream::raw_put(const nsl::byte_t *src,nsl::size_t sz)
/*! @brief Put raw memory data into the stream.

    raw_put() copies sz bytes from the src buffer into the stream, beginning at
    the current put position.

    @param src  Source buffer, may be null.
    @param sz   Source length, bytes.
*/
{
  if(0 == src)
    return;
  
  // Make sure we grow enough.
  while(size() + sz > capacity())
    nExpand();

  // Copy.
  memcpy(mPutPtr,src,sz);
  mPutPtr += sz;
}


/*  _________________________________________________________________________ */
void nsl::bstream::nExpand(void)
/*! @brief Increases the capacity of the stream.

    This function is called when the stream needs to grow. It grows the stream's
    capacity by a factor specified by the internal constant
    ikDefaultResizeFactor. It does not invalidate the contents of the stream,
    but it does invalidate any pointers (except the object's own get and set
    pointers) that point at or into the stream.
*/
{
nsl::byte_t   *tNew = new nsl::byte_t[ikDefaultResizeFactor * mCapacity];
nsl::offset_t  tPutOff = scast< nsl::offset_t >((mPutPtr - mData) - (mGetPtr - mData));

  memset(tNew,0,(ikDefaultResizeFactor * mCapacity));
  memcpy(tNew,mGetPtr,size());
  
  mCapacity *= ikDefaultResizeFactor;
  delete[] mData;
  mData = tNew;
  mPutPtr = mData + tPutOff;
  mGetPtr = mData;
}