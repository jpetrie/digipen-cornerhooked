/*!
	@file	Spring.h
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#pragma once

#ifndef	_SPRING_H_
#define	_SPRING_H_

namespace Physics
{
	class Engine;
	class RigidBody;

	class Spring
	{
	public:
		friend class Engine;

		Spring()
		{
			mStiffness		= k1;
			mDamping		= k1;
			mRestLength		= k1;
			mPrevLength		= k1;
			mPos1			= vZero;
			mPos2			= vZero;
			mCenterAttach1	= true;
			mCenterAttach2	= true;
			mCompressible	= true;
		}
		~Spring() {}

		bool mCompressible;
		Real mStiffness;
		Real mDamping;
		Real mRestLength;

		uint32_t mBody1, mBody2;
		Vector3D mPos1, mPos2;

	private:
		bool		mCenterAttach1, mCenterAttach2;	///< If true, no torque applied.
		RigidBody	*mBodyPtr1, *mBodyPtr2;			///< Points to connecting bodies.
		Real		mPrevLength;					///< Length from prev timestep.

	};
}

#endif