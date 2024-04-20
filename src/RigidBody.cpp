/*!
	@file	RigidBody.cpp
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"
#include "game.h"
#include "MathDefs.h"
#include "PhysicsEngine.h"
#include "RigidBody.h"
#include "CollisionEngine.h"
#include "profiler.h"

static void CapVectorNorm(Geometry::Vector3D &v, float max)
{
	if(v.length() > max)
		v = max * v.normal();
}

namespace Physics
{
/*!
 @return 
*//*__________________________________________________________________________*/
RigidBody::RigidBody() : mActive(true), mSpinnable(false), mTranslatable(false),
	mCollidable(false), mCollideGeom(0), mCollided(false)
{
	SetDefaults();
}
/*!
 @return 
*//*__________________________________________________________________________*/
RigidBody::~RigidBody()
{
	delete mCollideGeom;
}

/*!
 @param void 
*//*__________________________________________________________________________*/
void RigidBody::SetDefaults(void)
{
	mLinVelDamp = Real(.9995);
	mAngVelDamp = Real(.9995);
	mMass		= k1;
	mMassInv	= k1;
	mExtent[0]	= kHalf;
	mExtent[1]	= kHalf;
	mExtent[2]	= kHalf;
	mSpinnable	= false;
	mStateT0.SetAtRest();
	mStateT1	= mStateT0;
	mAccum.Clear();
	mCollided	= false;
	mAccelPrev	= vZero;
}

/*!
 @param IKind 
*//*__________________________________________________________________________*/
void RigidBody::InertiaKind(Physics::eInertiaKind IKind)
{
	mInertiaKind = IKind;
	CalculateInertiaTensor();
}

/*!
 @param mass 
*//*__________________________________________________________________________*/
void RigidBody::Mass(Real mass)
{
	mMass = mass;
	mMassInv = k1 / mass;
	CalculateInertiaTensor();
}

/*!
 @param void 
*//*__________________________________________________________________________*/
void RigidBody::CalculateInertiaTensor(void)
{
	if(mInertiaKind == Physics::kI_Sphere)
	{
		mInertiaTensorInv[0] = 5.f / (2.f * mMass * Math::Sqr((Real)mExtent[0])); // (2/5)mr^2
		return;
	}
}

/*!
 @param void 
 @return 
*//*__________________________________________________________________________*/
bool RigidBody::ResetForNextTimeStep(void)
{
	bool fellAsleep = false;

	mStateT0 = mStateT1;

	if(mTranslatable)
	{
		if(mCollided)
		{
			mAccum.mForce += (Real(-0.95) * mStateT0.mVelocity.length()) * mStateT0.mVelocity;
		}
		else
		{
			Real forceSquared = (Real)(mAccum.mForce * mAccum.mForce);
			if(forceSquared < kEpsilon)
			{
				mStateT0.mVelocity *= mLinVelDamp;
			}
		}
	}
	///@todo Change gravity from constant value;
	if(mStateT0.mVelocity.length() < (2.f * kGravity * kEpsilon))
		fellAsleep = true;

	if(mSpinnable)
	{
		Real aDamp = -mMass;// * 20;// * 2;// / 5;// * mAngVelDamp;
		mAccum.mTorque = aDamp * mStateT0.mAngularVelocity.normal();
	}

	mCollided = false;

	return fellAsleep;
}

/*!
 @param dt 
 @param gravity 
 
 note: Call before spring forces are calculated.
*//*__________________________________________________________________________*/
void RigidBody::Integrate1(Real dt)
{
 //   ProfileFn;
	//if(mTranslatable)
	//{
	//	mStateT1.mVelocity += (dt * kHalf) * mAccelPrev;
	//	mStateT1.mPosition += dt * mStateT1.mVelocity;
	//	mStateT1.mPosition += (dt * dt * kHalf) * mAccelPrev;
	//}
	//if(mSpinnable)
	//{
	//	mStateT1.mAngularMomentum += (dt * kHalf) * mTorquePrev;
	//	if(mInertiaKind == Physics::kI_Sphere)
	//	{
	//		mStateT1.mAngularVelocity = mInertiaTensorInv[0] * mStateT1.mAngularMomentum.normal();
	//	}
	//	else
	//	{
	//		for(int i = 0; i < 3; ++i)
	//			mStateT1.mAngularVelocity[i] = mInertiaTensorInv[i] * mStateT1.mAngularMomentum[i];
	//	}
	//	Quaternion q = mStateT1.mOrientation;
	//	q.Normalize();
	//	mStateT1.mOrientation += q * mStateT1.mAngularVelocity.normal() * static_cast< float >(kHalf * dt);// * ~q;
	//		//= InputAngularVelocity(dt, mStateT1.mAngularVelocity);
	//	mStateT1.mOrientation.Normalize();
	//}

    if(mTranslatable)
    {
        Geometry::Vector3D A = mAccum.mForce * MassInv();
        Geometry::Vector3D F;
        Geometry::Vector3D k1, k2, k3, k4;

        k1 = A * dt;

        F = F + k1 * .5f;
        A = F * MassInv();
        k2 = A * dt;

        F = F + k2 * .5f;
        A = F * MassInv();
        k3 = A * dt;

        F = F + k3;
        A = F * MassInv();
        k4 = A * dt;
        
        mStateT1.mVelocity += (k1 + k2 + k3 + k4) * (1.f/6.f);//(dt * kHalf) * mAccelPrev;
		mStateT1.mPosition += dt * mStateT1.mVelocity;
    }
    if(mSpinnable)
    {
        Geometry::Vector3D A = mAccum.mTorque * mInertiaTensorInv[0];
        Geometry::Vector3D F;
        Geometry::Vector3D k1, k2, k3, k4;

        k1 = A * dt;

        F = F + k1 * .5f;
        A = F * mInertiaTensorInv[0];
        k2 = A * dt;

        F = F + k2 * .5f;
        A = F * mInertiaTensorInv[0];
        k3 = A * dt;

        F = F + k3;
        A = F * mInertiaTensorInv[0];
        k4 = A * dt;

        mStateT1.mAngularVelocity += (k1 + k2 + k3 + k4) * (1.f/6.f);
        Quaternion q = mStateT1.mOrientation;
	    q.Normalize();
	    mStateT1.mOrientation += q * mStateT1.mAngularVelocity.normal() * static_cast< float >(kHalf * dt);// * ~q;
	//		//= InputAngularVelocity(dt, mStateT1.mAngularVelocity);
	    mStateT1.mOrientation.Normalize();
    }

	
}

/*!
 @param dt 
 @param gravity 
 
 note: Call before spring forces are calculated.
*//*__________________________________________________________________________*/
void RigidBody::Integrate2(Real dt, Vector3D gravity)
{
    ProfileFn;
	if(mTranslatable)
	{
		mAccelPrev = mMassInv * mAccum.mForce;
		if(mGravity)
			mAccelPrev += gravity;

		mStateT1.mVelocity += (dt * kHalf) * mAccelPrev;
		mAccum.mForce = vZero;
	}

	if(mSpinnable)
	{
		mTorquePrev = mAccum.mTorque;
		mStateT1.mAngularMomentum += (dt * kHalf) * mAccum.mTorque.normal();
		mAccum.mTorque = vZero;
	}
	// cap the angular and the linear velocity
    CapVectorNorm(mStateT1.mVelocity, Game::Get()->GetPhysics()->GetMaxLinearVelocity());
    CapVectorNorm(mStateT1.mAngularVelocity, Game::Get()->GetPhysics()->GetMaxAngularVelocity());
    CapVectorNorm(mStateT1.mAngularMomentum, Game::Get()->GetPhysics()->GetMaxAngularMomentum());
	
}

}
