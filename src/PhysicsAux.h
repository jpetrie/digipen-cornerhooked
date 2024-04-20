/*!
	@file		PhysicsAux.h
	@date		05-29-2004
	@author	Scott

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#pragma once

#ifndef __PHYSICS_AUX_H__
#define __PHYSICS_AUX_H__

#include <map>
#include "CollisionEngine.h"

namespace Physics
{
	class RigidBody;
	class Spring;

	typedef std::map<int, RigidBody *>	RigidBodyMap;
	typedef std::map<int, Spring *>		SpringMap;	
	typedef std::map< int, PhysicsCB > CallbackMap;


/*!
	@class		PhysicsEngine
	@ingroup	Physics Engine Proto
	@date		05-04-2004
	@author	Scott
*//*__________________________________________________________________________*/
	class AuxEngine
	{
	public:
		/// Collision Engine requires a pointer to its parent for callback purposes.

#pragma warning(disable:4355)	// 'this' : used in base member initializer list
		AuxEngine() :mCollisionEngine(this)
#pragma warning( default : 4355 )

		{
			mGravity = vZero;
		}
		~AuxEngine() {}

		Real				mMinTimeStep;
		Vector3D			mGravity;
		RigidBodyMap		mBodies;
		SpringMap			mSprings;
		Collision::Engine	mCollisionEngine;
		CallbackMap			mCallbacks;
	};
}

#endif
