/*!
	@file	Quaternion.h
	@author	Scott Smith
	@date	April 23, 2004

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

/******************************************************************************
																		guard
*******************************************************************************/

#ifndef __C_QUATERNION_H__
#define __C_QUATERNION_H__

#include "Geometry.hpp"
#include "Matrix.hpp"
#include "MathDefs.h"

using Geometry::Vector4D;
using Geometry::Vector3D;
using Geometry::Matrix4x4;

/*!
 @class		Quaternion
 @ingroup	collisions
 @date		04-23-2004
 @author	Scott
*//*__________________________________________________________________________*/
class Quaternion
{
public:
	
	// constructor
	Quaternion(float W = 1.f, float X = 0.f, float Y = 0.f, float Z = 0.f);

	// arithmetic operators
	Quaternion operator+=(const Quaternion &rhs);
	Quaternion operator-=(const Quaternion &rhs);
	Quaternion operator*=(const float s);
	Quaternion operator* (const Quaternion &rhs);
	Quaternion operator* (const Quaternion &rhs)const;
	Quaternion operator/=(const float s);
	Quaternion operator~(void);
	Quaternion operator~(void)const;
	Quaternion operator-(void);
	Quaternion operator-(void)const;

	// comparison operators
	bool operator==(const Quaternion &rhs);
	bool operator==(const Quaternion &rhs)const;
	bool operator!=(const Quaternion &rhs);
	bool operator!=(const Quaternion &rhs)const;
	
	// access functions
	float & operator[](const int i);
	float & operator[](const int i)const;
	float W(void);
	float W(void)const;
	float X(void);
	float X(void)const;
	float Y(void);
	float Y(void)const;
	float Z(void);
	float Z(void)const;

	// utility
	float		Dot (const Quaternion &rhs) const;	// dot product
	float		Magnitude(void);
	Vector3D	GetVector(void);
	float		GetScalar(void);
	float		GetAngle(void);
	Vector3D	GetAxis(void);
	Quaternion	RotateQ(const Quaternion &rhs);
	Vector3D	RotateV(const Vector3D &rhs);
	Vector3D	GetEulerAngles(void);
	void		Normalize();
	Matrix4x4	Basis(void);
	
	// data members
	Vector4D v;

};

Quaternion operator*(Quaternion q, Vector3D v);
Quaternion operator*(Vector3D v, Quaternion q);
Quaternion MakeFromEuler(float x, float y, float z);
Quaternion InputAngularVelocity(Real dt, const Vector3D v);



#endif