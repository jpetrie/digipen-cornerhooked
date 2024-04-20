/*! ========================================================================

      @file   nsl_random.h
      @brief  Interface to random number generator class.

    ======================================================================== */
    
/*                                                                   guard
---------------------------------------------------------------------------- */
#pragma once

/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class random
/*! @brief Random number generator.
*/
{
  public:
    // ct and dt
     random(void);             //!< Default constructor.
     random(unsigned long s);  //!< Constructor.
    ~random(void);             //!< Destructor.

    // seeding
    void seed(unsigned long s);  //!< Seed the generator.
    
    // generation
    long  rand_int(void);                   //!< Generate an integer in [0,0xFFFFFFFF].
    long  rand_int(long min,long max);      //!< Generate an integer in [min,max].
    float rand_float(void);                 //!< Generate a float in [0,1].
    float rand_float(float min,float max);  //!< Generate a float in [min,max].
    bool  rand_bool(void);                  //!< Generate a bool in [false,true].
    
    
  private:
    // data members
    unsigned long  *mMT;
    int             mMTI;
    unsigned long   mMag[2];
};