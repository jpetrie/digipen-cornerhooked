/*! ========================================================================

      @file   nsl_random.cpp
      @brief  Implementation of random number generator class.

    ======================================================================== */

#include "main.h"

/*    
      Portions of this code (the seed() and rand_int() functions) were
      written by Makoto Matsumoto, Takuji Nishimura, and other contributors
      to the Mersenne Twister project; the following copyright and
      disclaimer applies to that code:
      
      ----------------------------------------------------------------------
      Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
      All rights reserved.                          

      Redistribution and use in source and binary forms, with or without
      modification, are permitted provided that the following conditions
      are met:

        1. Redistributions of source code must retain the above copyright
            notice, this list of conditions and the following disclaimer.

        2. Redistributions in binary form must reproduce the above copyright
            notice, this list of conditions and the following disclaimer in the
            documentation and/or other materials provided with the distribution.

        3. The names of its contributors may not be used to endorse or promote 
            products derived from this software without specific prior written 
            permission.

      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
      A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
      CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
      EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
      PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
      PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
      LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
      NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

      Any feedback is very welcome.
      http://www.math.keio.ac.jp/matumoto/emt.html
      email: matumoto@math.keio.ac.jp
      ----------------------------------------------------------------------
      
      The original code was modifed slightly in terms of variable names,
      placement of brackets and whitespace, et cetera.
*/
    
/*                                                                  includes
---------------------------------------------------------------------------- */

#include "nsl_random.h"

#include <iostream>
#include <cmath>

/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const int            kN           = 624;
  const int            kM           = 397;
  const unsigned long  kMatrixA     = 0x9908b0dfUL;  // Constant vector a.
  const unsigned long  kUpperMask   = 0x80000000UL;  // Most significant W-R bits.
  const unsigned long  kLowerMask   = 0x7fffffffUL;  // Least significant R bits.
  const int            kDefaultSeed = 5489UL;        // Used if generating before a seed call.      
};


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
random::random(void)
/*! 
*/
: mMT(new unsigned long[kN]),
  mMTI(kN + 1)
{
  mMag[0] = 0x0UL;
  mMag[1] = kMatrixA;
}


/*  ________________________________________________________________________ */
random::random(unsigned long s)
/*! Construct with a specified seed value.

    @param s  Seed value. Cannot be zero.
*/
: mMT(new unsigned long[kN]),
  mMTI(kN + 1)
{
  mMag[0] = 0x0UL;
  mMag[1] = kMatrixA;
  
  // Seed generator.
  seed(s);
}


/*  ________________________________________________________________________ */
random::~random(void)
/*! 
*/
{
  delete[] mMT;
}


/*  ________________________________________________________________________ */
void random::seed(unsigned long s)
/*! 
    @param s  Seed value. Cannot be zero.
*/
{
  mMT[0] = s & 0xffffffffUL;
  for(mMTI = 1; mMTI < kN; mMTI++)
  {
    // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
    // In the previous versions, MSBs of the seed affect only MSBs of the array mMT[].                        
    // (Modified 2002/01/09 by Makoto Matsumoto).             
    mMT[mMTI] = (1812433253UL * (mMT[mMTI-1] ^ (mMT[mMTI-1] >> 30)) + mMTI); 

    // For 32-bit+ machines.
    mMT[mMTI] &= 0xffffffffUL;
  }
}


/*  ________________________________________________________________________ */
long random::rand_int(void)
/*! 
    @return
    A random integer in the interval [0,0xFFFFFFFF].
*/
{
unsigned long  y;

  if(mMTI >= kN)
  // Generate N words at one time.
  {
  int  kk;
  
    // If seed() has not been called, a default initial seed is used.
    if(mMTI == kN + 1)
      seed(kDefaultSeed);

    for(kk = 0; kk < kN - kM; kk++)
    {
      y       = (mMT[kk] & kUpperMask) | (mMT[kk + 1] & kLowerMask);
      mMT[kk] = mMT[kk + kM] ^ (y >> 1) ^ mMag[y & 0x1UL];
    }
    for(; kk < kN - 1; kk++)
    {
      y       = (mMT[kk] & kUpperMask) | (mMT[kk + 1] & kLowerMask);
      mMT[kk] = mMT[kk + (kM - kN)] ^ (y >> 1) ^ mMag[y & 0x1UL];
    }
    
    y           = (mMT[kN - 1] & kUpperMask) | (mMT[0] & kLowerMask);
    mMT[kN - 1] = mMT[kM - 1] ^ (y >> 1) ^ mMag[y & 0x1UL];
    mMTI        = 0;
  }

  y = mMT[mMTI++];

  // Tempering.
  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= (y >> 18);

  return (static_cast< long >(y));
}


/*  ________________________________________________________________________ */
long random::rand_int(long min,long max)
/*! 
    @param min  Minimum random value. Must be less than max.
    @param max  Maximum random value. Must be greater than min.
    
    @return
    A random integer in the interval [min,max].
*/
{
  return (abs(rand_int()) % abs(max - min) + min);
}


/*  ________________________________________________________________________ */
float random::rand_float(void)
/*! 
    @return
    A random float in the interval [0,1].
*/
{
  // An integer divided by 2^32-1.
  return (static_cast< unsigned long >(rand_int()) * (1.0f / 4294967295.0f));  
}


/*  ________________________________________________________________________ */
float random::rand_float(float min,float max)
/*! 
    @param min  Minimum random value. Must be less than max.
    @param max  Maximum random value. Must be greater than min.
    
    @return
    A random float in the interval [min,max].
*/
{
  return ((rand_float() * fabs(max - min)) + min);
}


/*  ________________________________________________________________________ */
bool random::rand_bool(void)
/*! 
    @return
    A random boolean value (true or false).
*/
{
  return (rand_int(0,1) == 0 ? true : false);
}


//
