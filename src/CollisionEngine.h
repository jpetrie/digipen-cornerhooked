/*!
	@file	CollisionEngine.h
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/
#pragma once

#ifndef	__COLLISIONENGINE_H__
#define	__COLLISIONENGINE_H__

#pragma warning( push, 3 )	// shut up the compiler warning 4702
#pragma warning( disable: 4702 )
#include <vector>
#include <list>
#include <map>
#pragma warning( pop )

#include "PhysicsDefs.h"
#include "MathDefs.h"
#include "RigidBody.h"

const int kCollisionCBSpherePlane	= 1;
const int kCollisionCBSphereSphere	= 2;
const int kCollisionCBSpherePocket	= 3;
const int kCallbackGoneStatic       = 4;

namespace Physics
{
	class AuxEngine;
}
namespace Collision
{
/*!
	 @class		Contact
	 @ingroup	Physics Engine Proto
	 @date		05-05-2004
	 @author	Scott
	*//*__________________________________________________________________________*/
	class Contact
	{
	public:
        Contact():mBody1(0), mBody2(0)	{}
		Vector3D	mPosition;
		Vector3D	mNormal;
		Real		mTime;
		Real		mDepth;
		Physics::RigidBody	*mBody1, *mBody2;
		uint32_t	mID1, mID2;
	};
}
typedef void (*PhysicsCB)(Collision::Contact*, Physics::RigidBody*, Physics::RigidBody* body2);

namespace Collision
{

/*!
 @class		Engine
 @ingroup	Physics Engine Proto
 @date		05-05-2004
 @author	Scott
*//*__________________________________________________________________________*/
class Engine
{
public:
	Engine(Physics::AuxEngine*);
	~Engine();
	void SetCapacity(int maxContacts);
	void Begin(void);
	Contact* TestCollision(Physics::RigidBody* body1, Physics::RigidBody* body2);
	void Clear(Contact* contact);
	void Resolve(Contact* contact);
	void End(void);
	std::vector< Contact * > mContacts;
	std::list< Contact * > mFreePool;
	Physics::AuxEngine*	mParentPE;
};

}

#endif