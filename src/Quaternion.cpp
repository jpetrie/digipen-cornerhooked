/*!
	@file	Quaternion.cpp
	@author	Scott Smith
	@date	April 23, 2004

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"
#include "Quaternion.h"

// constructor
/*!
	@param W Scalar quantity for the quaternion.
	@param X Specifies the magnitude of the vector quantity along e1.
	@param Y Specifies the magnitude of the vector quantity along e2.
	@param Z Specifies the magnitude of the vector quantity along e3.

	constructor.
*//*__________________________________________________________________________*/
Quaternion::Quaternion(float W, float X, float Y, float Z):v(W, X, Y, Z) {}

////////////////////////////////////////////////////////////////////////////////
// arithmetic operators
////////////////////////////////////////////////////////////////////////////////
/*!
 @param		rhs A quaternion.
 @return		The sum of this quaternion and rhs.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator+=(const Quaternion &rhs)
{
	v += rhs.v;
	return *this;
}

/*!
 @param		rhs A quaternion.
 @return		The difference between this quaternion and rhs.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator-=(const Quaternion &rhs)
{
	v -= rhs.v;
	return *this;
}

/*!
 @param s	A scaling factor.
 @return	A scaling of this quaternion.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator*=(const float s)
{
	v *= s;
	return *this;
}

/*!
 @param s	A scaling factor.
 @return	A scaled version of this quaternion.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator/=(const float s)
{
   for(int i = 0; i < 4; ++i)
		v[i] /= s;
	return *this;
}

/*!
 @param		rhs A quaternion.
 @return		The sum of two quaternions.
 @brief			Multiplication is not usually commutative, so in most cases p*q != q*p.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator* (const Quaternion &rhs)
{
	Quaternion ret;

	ret.v[0] = v[0] * rhs.v[0] - v[1] * rhs.v[1] - v[2] * rhs.v[2] - v[3] * rhs.v[3];
    ret.v[1] = v[0] * rhs.v[1] + v[1] * rhs.v[0] + v[2] * rhs.v[3] - v[3] * rhs.v[2];
    ret.v[2] = v[0] * rhs.v[2] + v[2] * rhs.v[0] + v[3] * rhs.v[1] - v[1] * rhs.v[3];
    ret.v[3] = v[0] * rhs.v[3] + v[3] * rhs.v[0] + v[1] * rhs.v[2] - v[2] * rhs.v[1];

    return ret;
}
/*!
 @param		rhs A quaternion.
 @return		The sum of two quaternions.
 @brief			Multiplication is not usually commutative, so in most cases p*q != q*p.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator* (const Quaternion &rhs)const
{
	Quaternion ret;

	ret.v[0] = v[0] * rhs.v[0] - v[1] * rhs.v[1] - v[2] * rhs.v[2] - v[3] * rhs.v[3];
    ret.v[1] = v[0] * rhs.v[1] + v[1] * rhs.v[0] + v[2] * rhs.v[3] - v[3] * rhs.v[2];
    ret.v[2] = v[0] * rhs.v[2] + v[2] * rhs.v[0] + v[3] * rhs.v[1] - v[1] * rhs.v[3];
    ret.v[3] = v[0] * rhs.v[3] + v[3] * rhs.v[0] + v[1] * rhs.v[2] - v[2] * rhs.v[1];

    return ret;
}
/*!
 @return	The conjugate of this quaternion. 
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator~(void)
{
	return Quaternion((float)v[3], (float)-v[0], (float)-v[1], (float)-v[2]);
}
/*!
 @return	The conjugate of this quaternion.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator~(void)const
{
	return Quaternion((float)v[3], (float)-v[0], (float)-v[1], (float)-v[2]);
}
/*!
 @return	The negation of this quaternion.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator-(void)
{
	return Quaternion((float)-v[3], (float)-v[0], (float)-v[1], (float)-v[2]);
}
/*!
 @return	The negation of this quaternion.
*//*__________________________________________________________________________*/
Quaternion Quaternion::operator-(void)const
{
	return Quaternion((float)-v[3], (float)-v[0], (float)-v[1], (float)-v[2]);
}
////////////////////////////////////////////////////////////////////////////////
// comparison operators
////////////////////////////////////////////////////////////////////////////////
/*!
 @param rhs	A quaternion. 
 @return	True if the two quaternions are equal, false otherwise.
*//*__________________________________________________________________________*/
bool Quaternion::operator==(const Quaternion &rhs)
{
	return (v == rhs.v);
}
/*!
 @param rhs	A quaternion. 
 @return	True if the two quaternions are equal, false otherwise.
*//*__________________________________________________________________________*/
bool Quaternion::operator==(const Quaternion &rhs)const
{
	return (v == rhs.v);
}
/*!
 @param	rhs	A quaternion.
 @return	True if the two quaternions are not equal, false otherwise.
*//*__________________________________________________________________________*/
bool Quaternion::operator!=(const Quaternion &rhs)
{
	return !(*this == rhs);
}
/*!
 @param	rhs	A quaternion.
 @return	True if the two quaternions are not equal, false otherwise.
*//*__________________________________________________________________________*/
bool Quaternion::operator!=(const Quaternion &rhs)const
{
	return !(*this == rhs);
}

////////////////////////////////////////////////////////////////////////////////
// access functions
////////////////////////////////////////////////////////////////////////////////
/*!
 @param i	An index into the quaternion, 0 - 3.
 @return	The component at the index.
*//*__________________________________________________________________________*/
float & Quaternion::operator[](const int i)
{
	return (float&)v[i];
}
/*!
 @param i	An index into the quaternion, 0 - 3.
 @return	The component at the index.
*//*__________________________________________________________________________*/
float & Quaternion::operator[](const int i)const
{
	return (float&)v[i];
}
/*!
 @return	The scalar quantity of the quaternion. 
*//*__________________________________________________________________________*/
float Quaternion::W(void)
{
	return (float)v[0];
}
/*!
 @return	The scalar quantity of the quaternion. 
*//*__________________________________________________________________________*/
float Quaternion::W(void)const
{
	return (float)v[0];
}
/*!
 @return	The x quantity of the quaternion's vector. 
*//*__________________________________________________________________________*/
float Quaternion::X(void)
{
	return (float)v[1];
}
/*!
 @return	The x quantity of the quaternion's vector. 
*//*__________________________________________________________________________*/
float Quaternion::X(void)const
{
	return (float)v[1];
}
/*!
 @return	The y quantity of the quaternion's vector. 
*//*__________________________________________________________________________*/
float Quaternion::Y(void)
{
	return (float)v[2];
}
/*!
 @return	The y quantity of the quaternion's vector. 
*//*__________________________________________________________________________*/
float Quaternion::Y(void)const
{
	return (float)v[2];
}
/*!
 @return	The z quantity of the quaternion's vector. 
*//*__________________________________________________________________________*/
float Quaternion::Z(void)
{
	return (float)v[3];
}
/*!
 @return	The z quantity of the quaternion's vector. 
*//*__________________________________________________________________________*/
float Quaternion::Z(void)const
{
	return (float)v[3];
}
////////////////////////////////////////////////////////////////////////////////
// utility
////////////////////////////////////////////////////////////////////////////////
/*!
 @param	rhs	A quaternion.
 @return	The dot product of two quaternions. 
*//*__________________________________________________________________________*/
float Quaternion::Dot (const Quaternion &rhs) const	// dot product
{
	float dot = 0.f;
	for(int i = 0; i < 4; ++i)
		dot += (float)(v[i] * rhs.v[i]);
	return dot;
}
/*!
 @param rhs A 3D vector.
 @return	The vector, rhs, rotated by the quaternion
*//*__________________________________________________________________________*/
Vector3D Quaternion::RotateV(const Vector3D &rhs)
{
	Quaternion t;
	t = *this * rhs * (~*this);
	return t.GetVector();
}
/*!
 @param	rhs A quaternion. 
 @return	The quaternion, rhs, rotated by the quaternion. 
*//*__________________________________________________________________________*/
Quaternion Quaternion::RotateQ(const Quaternion &rhs)
{
	return *this * rhs * (~*this);
}

/*!
 @param q A quaternion. 
 @param v A vector.
 @return A quaternion, the product of q and v.
*//*__________________________________________________________________________*/
Quaternion operator*(Quaternion q, Vector3D v)
{
	return Quaternion( (float)-(q.v[1] * v[0] + q.v[2] * v[1] + q.v[3] * v[2]),
						(float)(q.v[0] * v[0] + q.v[2] * v[2] + q.v[3] * v[1]),
						(float)(q.v[0] * v[2] + q.v[3] * v[0] + q.v[1] * v[2]),
						(float)(q.v[0] * v[2] + q.v[1] * v[1] + q.v[2] * v[0]));
}
/*!
 @param v A vector. 
 @param q A quaternion.
 @return A quaternion, the product of v and q.
*//*__________________________________________________________________________*/
Quaternion operator*(Vector3D v, Quaternion q)
{
	return Quaternion( (float)-(q.v[1] * v[0] + q.v[2] * v[1] + q.v[3] * v[2]),
						(float)(q.v[0] * v[0] + q.v[2] * v[2] + q.v[2] * v[2]),
						(float)(q.v[0] * v[2] + q.v[1] * v[2] + q.v[3] * v[0]),
						(float)(q.v[0] * v[2] + q.v[2] * v[0] + q.v[1] * v[1]));
}
/*!
 @return	The magnitude of the quaternion.
*//*__________________________________________________________________________*/
float Quaternion::Magnitude(void)
{
	float ret = 0.f;
	for(int i = 0; i < 4; ++i)
		ret += (float)(v[i] * v[i]);
	return sqrt(ret);
}
/*!
 @return	The vector quantity of the quaternion.
*//*__________________________________________________________________________*/
Vector3D Quaternion::GetVector(void)
{
	return Vector3D(v[1], v[2], v[3]);
}
/*!
 @return	The scalar component of the quaternion. 
*//*__________________________________________________________________________*/
float Quaternion::GetScalar(void)
{
	return (float)v[0];
}
/*!
 @return	The angle the quaternion represents. 
*//*__________________________________________________________________________*/
float Quaternion::GetAngle(void)
{
	return (float)(2.f * acos(v[0]));
}
/*!
 @return	The axis of rotation, as a vector. 
*//*__________________________________________________________________________*/
Vector3D Quaternion::GetAxis(void)
{
	if(v.length() <= std::numeric_limits<float>::epsilon())
		return Vector3D();
	else
		return GetVector().normal();
}
void Quaternion::Normalize(void)
{
	v = v * Math::Inverse((float)v.length());
}
/*!
 @return	A vector with components representing the roll, pitch, and yaw. 
*//*__________________________________________________________________________*/
Vector3D Quaternion::GetEulerAngles(void)
{
	float r11, r12, r13, r21, r31, r32, r33;
	float q00, q11, q22, q33;
	float temp;
	Vector3D u;
	q00 = (float)v[0] * (float)v[0];
	q11 = (float)v[1] * (float)v[1];
	q22 = (float)v[2] * (float)v[2];
	q33 = (float)v[3] * (float)v[3];

	r11 = q00 + q11 - q22 - q33;
	r21 = 2.f * (float)(v[1] * v[2] + v[0] * v[3]);
	r31 = 2.f * (float)(v[1] * v[3] - v[0] * v[2]);
	r32 = 2.f * (float)(v[2] * v[3] + v[0] * v[1]);
	r33 = q00 - q11 - q22 + q33;

	temp = fabs(r31);
	if(temp > 0.999999f)
	{
		r12 = 2.f * (float)(v[1] * v[2] - v[0] * v[3]);
		r13 = 2.f * (float)(v[1] * v[3] + v[0] * v[2]);
		u[0] = Math::RadToDeg(0.f);									// roll
		u[1] = Math::RadToDeg( float(-(kPI / 2.f) * r31/temp) );			// pitch
		u[2] = Math::RadToDeg( atan2(-r12, -r31 * r13) );			// yaw
		return u;
	}

	u[0] = Math::RadToDeg( atan2(r32, r33) );					// roll
	u[1] = Math::RadToDeg( asin(-r31) );						// pitch
	u[2] = Math::RadToDeg( atan2(r21, r11) );					// yaw
	return u;
}
/*!
 @param x Roll
 @param y Pitch
 @param z Yaw
*//*__________________________________________________________________________*/
Quaternion MakeFromEuler(float x, float y, float z)
{
	Quaternion q;
	float roll	= float(Math::DegToRad(x));
	float pitch = float(Math::DegToRad(y));
	float yaw	= float(Math::DegToRad(z));
	float croll, cpitch, cyaw, sroll, spitch, syaw;
	float cyawcpitch, syawspitch, cyawspitch, syawcpitch;

	cyaw = cos(.5f * yaw);
	cpitch = cos(.5f * pitch);
	croll = cos(.5f * roll);
	syaw = sin(.5f * yaw);
	spitch = sin(.5f * pitch);
	sroll = sin(.5f * roll);

	cyawcpitch = cyaw * cpitch;
	syawspitch = syaw * spitch;
	cyawspitch = cyaw * spitch;
	syawcpitch = syaw * cpitch;

	q.v[0] = cyawcpitch * croll + syawspitch * sroll;
	q.v[1] = cyawcpitch * sroll - syawspitch * croll;
	q.v[2] = cyawspitch * croll + syawcpitch * sroll;
	q.v[3] = syawcpitch * croll - cyawspitch * sroll;

	return q;

}

/*!
 @param dt 
 @param v 
 @return 
*//*__________________________________________________________________________*/
Quaternion InputAngularVelocity(Real dt, const Geometry::Vector3D v)
{
	Quaternion vel(float(k0), (float)v[0], (float)v[1], (float)v[2]);
	vel = vel * v;
	Real scale = dt * kHalf;
	vel.v[0] *= scale;
	vel.v[1] *= scale;
	vel.v[2] *= scale;
	vel.v[3] *= scale;
	return vel;
}
/*!
 @param void 
 @return 
*//*__________________________________________________________________________*/
Matrix4x4	Quaternion::Basis(void)
{
	Matrix4x4 ret;
	Real x = X();
	Real y = Y();
	Real z = Z();
	Real w = W();
	Real x2 = x + x;
	Real y2 = y + y;
	Real z2 = z + z;
	Real xx = x * x2;
	Real xy = x * y2;
	Real xz = x * z2;
	Real yy = y * y2;
	Real yz = y * z2;
	Real zz = z * z2;
	Real wx = w * x2;
	Real wy = w * y2;
	Real wz = w * z2;
	ret[0][0] = k1 - (yy + zz);
	ret[0][1] = xy - wz;
	ret[0][2] = xz + wy;
	ret[1][0] = xy + wz;
	ret[1][1] = k1 - (xx + zz);
	ret[1][2] = yz - wx;
	ret[2][0] = xz - wy;
	ret[2][1] = yz + wx;
	ret[2][2] = k1 - (xx + yy);

	return ret;
}

