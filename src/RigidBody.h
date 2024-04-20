/*!
	@file	RigidBody.h
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#pragma once

#ifndef	__RIGIDBODY_H__
#define	__RIGIDBODY_H__

#include "PhysicsDefs.h"
#include "Quaternion.h"

using Geometry::Vector3D;

namespace
{
	const Quaternion qZero = Quaternion();
	const Vector3D vZero = Geometry::Vector3D();
}

const Real CoeffRestSP = 0.6f;
const Real CoeffRestSS = 0.8f;

namespace Physics
{
	/*!
	 @class		RigidAccumulator
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class RigidAccumulator
	{
	public:
		RigidAccumulator() { Clear(); }
		~RigidAccumulator() {}
		void Clear(void)
		{
			mForce = vZero;
			mTorque = vZero;
		}
		void AddForce(Vector3D f)	{ mForce += f; }
		void AddTorque(Vector3D t)	{ mTorque += t; }

		Vector3D mForce, mTorque;
	};

	/*!
	 @class		DynamicTraits
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class DynamicTraits
	{
	public:
		DynamicTraits() { SetAtRest(); }
		~DynamicTraits() {}
		DynamicTraits & operator=(const DynamicTraits& rhs)
		{
			mOrientation = rhs.mOrientation;
			mPosition = rhs.mPosition;
			mVelocity = rhs.mVelocity;
			mAngularVelocity = rhs.mAngularVelocity;
			mAngularMomentum = rhs.mAngularMomentum;

			return *this;
		}
		void SetAtRest(void)
		{
			mOrientation = qZero;
			mOrientation[0] = float(k1);
			mPosition = vZero;
			mVelocity = vZero;
		}

		Quaternion	mOrientation;
		Vector3D	mPosition;
		Vector3D	mVelocity;
		Vector3D	mAngularVelocity;
		Vector3D	mAngularMomentum;
	};

	/*!
	 @class		RigidBody
	 @ingroup	Physics Engine Proto
	 @date		05-04-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class RigidBody
	{
	public:
		RigidBody();
        virtual ~RigidBody();
		
		virtual bool	ResetForNextTimeStep(void);
		void			Integrate1(Real dt);
		void			Integrate2(Real dt, Vector3D gravity);
		virtual void	SetCollisionObject(GeometryType * collide) { mCollideGeom = collide; }
		virtual void	InertiaKind(eInertiaKind);
		eInertiaKind	InertiaKind(void) const		{ return mInertiaKind; }
		virtual void	Mass(Real mass);
		Real			Mass(void)const				{ return mMass; }
		Real			MassInv(void)const			{ return mMassInv; }
		void			CalculateInertiaTensor();
		virtual void	Renormalize(void) {}
		void			SetDefaults(void);
		void			Gravity(bool val)			{ mGravity = val; }
		void			Active(bool val)			{ mActive = val; }
		void			Collidable(bool val)		{ mCollidable = val; }
		void			Spinnable(bool val)			{ mSpinnable = val; }
		void			Translatable(bool val)		{ mTranslatable = val; }
		bool			Gravity(void)				{ return mGravity; }
		bool			Active(void)				{ return mActive; }
		bool			Collidable(void)			{ return mCollidable; }
		bool			Spinnable(void)				{ return mSpinnable; }
		bool			Translatable(void)			{ return mTranslatable; }	
		void			AngVelocityDamp(Real val)	{ mAngVelDamp = val * Real(0.995); }
		Real			AngVelocityDamp(void)		{ return mAngVelDamp * Real(1.f / .995f); }
		void			LinVelocityDamp(Real val)	{ mLinVelDamp = val * Real(0.995); }
		Real			LinVelocityDamp(void)		{ return mLinVelDamp * Real(1.f / .995f); }

	public:
		RigidAccumulator	mAccum;				///< Accumulated forces and torques.
		DynamicTraits		mStateT0;			///< State at beginning of time step.
		DynamicTraits		mStateT1;			///< State at end of time step.
		Vector3D			mExtent;			///< Extent in each direction from local origin.
		GeometryType*		mCollideGeom;		///< Pointer to collision geometry
		Vector3D			mInertiaTensorInv;	///< Diagonal of inverse inertia matrix.
		bool				mCollided;			///< Indicates a collision during the frame.

	protected:
		Real				mLinVelDamp;		///< Linear damping coeff.
		Real				mAngVelDamp;		///< Angular damping coeff.
		Vector3D			mAccelPrev;			///< Acceleration from previous frame.
		Vector3D			mTorquePrev;		///< Torque from previous frame.
		eInertiaKind		mInertiaKind;		///< Type of inertia calculations to perform.
		bool				mActive;			///< Indicates if the body participates in the simulation.
		bool				mSpinnable;			///< Can spin.
		bool				mTranslatable;		///< Can move.	
		bool				mCollidable;		///< Can participate in collisions.
		bool				mGravity;			///< Affected by gravity.
		Real				mMass;				///< Object mass. Must be > .2 for accuracy (float resolution).
		Real				mMassInv;			///< Inverse of the body's mass.
	};
} // namespace Physics

#endif
