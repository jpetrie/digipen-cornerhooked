/*! =========================================================================  
      
      @file    perlin.h
      @author  jmp
      @brief   Interface to Perlin noise generator.
      
    ========================================================================= */


/*                                                                      guard
----------------------------------------------------------------------------- */

#ifndef _PERLIN_H_
#define _PERLIN_H_


/*                                                                    structs
----------------------------------------------------------------------------- */

struct perlin_def_random
//! Default random numbner generator policy.
{
  void  seed(unsigned long s)  { srand(s); }
  float random(void)           { return (static_cast< float>(rand()) / (RAND_MAX / 2.0f) - 1.0f); }
};

/*                                                                    classes
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
template< unsigned int D_ = 2,typename G_ = perlin_def_random,unsigned int S_ = 256 >
class perlin
/*! Policy-based Perlin noise generator.

    @param D_  The dimension of the input. Defaults to (and must be) 2.
    @param G_  The RNG policy type. Defaults to perlin_def_random.
               Must implement seed() to seed the generator and random() to
               return a number in [-1..1].
    @param S_  The size of the permutation table. Defaults to 256.
*/
{
  public:
    // ct and dt
    perlin(unsigned int seed = 0);
    ~perlin(void);
    
    // noise function
    float noise(float *coords);
    
  private: 
    G_     mRNG;                //!< Random number generator.
    int    mPermTable[S_];      //!< Permutation table.
    float  mGradTable[D_][S_];  //!< Gradient vector tables.
};


/*                                                             implementation
----------------------------------------------------------------------------- */

#include "perlin.inl"


#endif  /* _PERLIN_H_ */