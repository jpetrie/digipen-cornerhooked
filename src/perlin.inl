/*! =========================================================================  
      
      @file    perlin.inl
      @author  jmp
      @brief   Implementation of Perlin noise generator.
      
    ========================================================================= */

/*                                                                  functions
----------------------------------------------------------------------------- */

/*  _________________________________________________________________________ */
template< unsigned int D_,typename G_,unsigned int S_ >
perlin< D_,G_,S_ >::perlin(unsigned int seed)
/*! Default constructor.

    @param seed  Generator seed. Defaults to 0.
*/
{
  mRNG.seed(seed);
  
  // Build permutation table.
  for(unsigned int i = 0; i < S_; ++i)
    mPermTable[i] = i;
  for(unsigned int i = 0; i < S_; ++i)
    std::swap(mPermTable[i],mPermTable[rand() % S_]);
  	
  // Build gradient vector tables.
  for(unsigned int d = 0; d < D_; ++d)
    for(unsigned int i = 0; i < S_; ++i)
      mGradTable[d][i] = mRNG.random();
}

/*  _________________________________________________________________________ */
template< unsigned int D_,typename G_,unsigned int S_ >
perlin< D_,G_,S_ >::~perlin(void)
/*! Destructor.
*/
{
}

/*  _________________________________________________________________________ */
template< unsigned int D_,typename G_,unsigned int S_ >
float perlin< D_,G_,S_ >::noise(float *coords)
/*! Noise function.

    @param coords  An array of input values, of dimension D_.
    
    @return
    A Perlin noise value in [0..1].
*/
{
  // Stupid-proofing.
  if(coords == 0)
    return (0.0f);
    
  // Compute the positions of the neighbors.
int  nx0 = static_cast< int >(::floorf(coords[0]));
int  nx1 = nx0 + 1;
int  ny0 = static_cast< int >(::floorf(coords[1]));
int  ny1 = ny0 + 1;

  // Get gradient vector indices.
int  i00 = mPermTable[(ny0 + mPermTable[nx0 % S_]) % S_];
int  i01 = mPermTable[(ny0 + mPermTable[nx1 % S_]) % S_];
int  i10 = mPermTable[(ny1 + mPermTable[nx0 % S_]) % S_];
int  i11 = mPermTable[(ny1 + mPermTable[nx1 % S_]) % S_];

  // Compute the internal vectors (from the neighbors to the input).
float  vx0 = coords[0] - ::floorf(coords[0]);
float  vx1 = vx0 - 1;
float  vy0 = coords[1] - ::floorf(coords[1]);
float  vy1 = vy0 - 1;

  // Compute the dot-product between the vectors and the gradients.
float  d00 = mGradTable[0][i00] * vx0 + mGradTable[1][i00] * vy0;
float  d01 = mGradTable[0][i01] * vx1 + mGradTable[1][i01] * vy0;
float  d10 = mGradTable[0][i10] * vx0 + mGradTable[1][i10] * vy1;
float  d11 = mGradTable[0][i11] * vx1 + mGradTable[1][i11] * vy1;

  // Utilize cubic interpolation to get the final value.
float  wx = (3 - 2 * vx0) * vx0 * vx0;
float  v0 = d00 - wx * (d00 - d01);
float  v1 = d10 - wx * (d10 - d11);
float  wy = (3 - 2 * vy0) * vy0 * vy0;
float  v  = v0 - wy * (v0 - v1);

  // v is in [-1..1], scale it up to [0..1].
  return ((v + 1.0f) / 2.0f);
}