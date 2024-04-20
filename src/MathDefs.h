/*!
	@file	MathDefs.h
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Math definitions.
______________________________________________________________________________
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */
#ifndef _MATHDEFS_H_
#define	_MATHDEFS_H_

#include <cmath>
#include <limits>
#include "StdTypes.h"

typedef float			Real;

const Real	kEpsilon	= std::numeric_limits< float >::epsilon();  ///< A really small value for testing against zero.
const Real	k0			= 0.0f;       ///< 0
const Real	k1			= 1.0f;       ///< 1
const Real	kN1			= -1.0f;      ///< -1
const Real	kHalf		= 0.5f;       ///< 1/2
const Real	kPI			= 3.141592f;  ///< PI
const Real  kMax        = std::numeric_limits< float >::max();
const Real  kMin        = std::numeric_limits< float >::min();

/*!
	@namespace Math
	@brief	Contains definitions for math functions.
*//*__________________________________________________________________________*/
namespace Math
{
	/*!
	 @param deg 
	 @return An equivalent angle in radians.
	______________________________________________________________________________*/
	inline Real DegToRad(const float deg)	{ return deg * kPI / 180.f; }
	/*!
	 @param rad 
	 @return An euivalent angle in degress.
	______________________________________________________________________________*/
	inline Real RadToDeg(const float rad)	{ return rad * 180.f / kPI; }
	/*!
	 @param r 
	 @return The value r, squared.
	______________________________________________________________________________*/
	inline Real Sqr(Real r)					{ return r * r; }
	/*!
	 @param r 
	 @return The square root of r.
	______________________________________________________________________________*/
	inline Real Sqrt(Real r)				{ return (Real)sqrt(r); }
	/*!
	 @param r 
	 @return 1 over the square root of r.
	______________________________________________________________________________*/
	inline Real SqrtInv(Real r)				{ return Real(k1 / sqrt(r)); }
	/*!
	 @param r 
	 @return The absolute value of r.
	______________________________________________________________________________*/
	inline Real	Abs(Real r)					{ return r > k0 ? r : -r; }
	/*!
	 @param r 
	 @return 1 over r.
	______________________________________________________________________________*/
	inline Real Inverse(Real r)				{ return r == 0.f ? 0.f : k1 / r; }
	/*!
	 @param a 
	 @param b 
	 @brief	Swaps a and b.
	______________________________________________________________________________*/
	inline void Swap(Real& a, Real& b)		{ Real temp = a; a = b; b = temp; }
}

#endif