/*!
 @file		Trig.cpp
 @ingroup	collisions
 @date		04-23-2004
 @author	Scott Smith

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"

#include "trig.h"

namespace Math
{
namespace
{
const float PI = 3.141592f;
}

float SinD(const float deg)
{
	return SinR(DegToRad(deg));
}
float SinR(const float rad)
{	
	float SqAngle = rad * rad;
    float ret = -(float)2.39e-08;
    ret *= SqAngle;
    ret += (float)2.7526e-06;
    ret *= SqAngle;
    ret -= (float)1.98409e-04;
    ret *= SqAngle;
    ret += (float)8.3333315e-03;
    ret *= SqAngle;
    ret -= (float)1.666666664e-01;
    ret *= SqAngle;
    ret += (float)1.0;
    ret *= rad;
    return ret;
}
float CosD(const float deg)
{	
	return CosR(DegToRad(deg));
}
float CosR(const float rad)
{
	float SqAngle = rad * rad;
    float ret = -(float)2.605e-07;
    ret *= SqAngle;
    ret += (float)2.47609e-05;
    ret *= SqAngle;
    ret -= (float)1.3888397e-03;
    ret *= SqAngle;
    ret += (float)4.16666418e-02;
    ret *= SqAngle;
    ret -= (float)4.999999963e-01;
    ret *= SqAngle;
    ret += (float)1.0;
    return ret;
}
float TanD(const float deg)
{
	return SinD(deg)/CosD(deg);
}
float TanR(const float rad)
{
	return SinR(rad)/CosR(rad);
}

float ASinD(const float val)
{
	return fmod(RadToDeg(ASinR(val)), 360.f); 
}
 float ASinR(const float val)
{
	float Root = Sqrt(((float)1.0)- val);
    float Ret = -(float)0.0187293;
    Ret *= val;
    Ret += (float)0.0742610;
    Ret *= val;
    Ret -= (float)0.2121144;
    Ret *= val;
    Ret += (float)1.5707288;
    Ret = PI/2.f - Root * Ret;
    return Ret;
}
float ACosD(const float val)
{
	return fmod(RadToDeg(ACosR(val)), 360.f); 
}
float ACosR(const float val)
{
	float Root = Sqrt(((float)1.0)- val);
    float ret = -(float)0.0187293;
    ret *= val;
    ret += (float)0.0742610;
    ret *= val;
    ret -= (float)0.2121144;
    ret *= val;
    ret += (float)1.5707288;
    ret *= Root;
    return ret;
}
float ATanD(const float val)
{
	return fmod(RadToDeg(ATanR(val)), 360.f); 
}
float ATanR(const float val)
{
	float Sqr = val * val;
    float ret = (float)0.0028662257;
    ret *= Sqr;
    ret -= (float)0.0161657367;
    ret *= Sqr;
    ret += (float)0.0429096138;
    ret *= Sqr;
    ret -= (float)0.0752896400;
    ret *= Sqr;
    ret += (float)0.1065626393;
    ret *= Sqr;
    ret -= (float)0.1420889944;
    ret *= Sqr;
    ret += (float)0.1999355085;
    ret *= Sqr;
    ret -= (float)0.3333314528;
    ret *= Sqr;
    ret += (float)1.0;
    ret *= val;
    return ret;
}

float Sqrt(const float value)
{
	float val = (float)value;
    float Half = 0.5f * val;
    int i  = *(int*)&val;
    i = 0x5f3759df - (i >> 1);
    val = *(float*)&i;
    val = val * (1.5f - Half * val * val);
	if(val == 0.f) return 0.f;
    return 1.f/(float)val;
}


float pi(void)
{
	return PI;
}

}