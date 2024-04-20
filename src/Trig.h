/*!
 @file		Trig.h
 @ingroup	collisions
 @date		04-23-2004
 @author	Scott Smith

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef __TRIG_H__
#define __TRIG_H__

namespace Math
{
float SinD(const float);
float SinR(const float);
float ASinD(const float);
float ASinR(const float);
float CosD(const float);
float CosR(const float);
float ACosD(const float);
float ACosR(const float);
float TanD(const float);
float TanR(const float);
float ATanD(const float);
float ATanR(const float);

float Sqrt(const float);

float DegToRad(const float);
float RadToDeg(const float);
float pi(void);
}

#endif
