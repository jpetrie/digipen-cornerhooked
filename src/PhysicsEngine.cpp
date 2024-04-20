/*!
	@file	PhysicsEngine.cpp
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/


#include "main.h"

#include "game.h"
#include "CollisionEngine.h"
#include "PhysicsEngine.h"
#include "PhysicsAux.h"
#include "RigidBody.h"
#include "Spring.h"
#include "Quaternion.h"
#include "log.h"
#include "profiler.h"
#include "Player.h"
#include "RuleSystem.h"

using Geometry::Vector4D;
using Geometry::Matrix4x4;
using Physics::AuxEngine;

namespace Physics
{
	typedef std::map<int, RigidBody *>	RigidBodyMap;
	typedef std::map<int, Spring *>		SpringMap;	
	typedef std::map< int, PhysicsCB > CallbackMap;
}

static uint32_t UniqueID(void)
{
	static uint32_t id = 0;
	return ++id;
}

namespace Physics
{
	struct CollisionSortPred
    {
        bool operator()(const Collision::Contact*lhs, const Collision::Contact*rhs)
        {
            return (lhs->mTime < rhs->mTime);
        }
    };

} // namespace Physics

void Physics::Engine::Disturb(void)
{
	Physics::Engine::mIsStatic = false;
}
/*!
 @return 
*//*__________________________________________________________________________*/
Physics::Engine::Engine(void): mIsStatic(false),mWasStatic(true)
{
    mMaxLinVel = GetConfigValue("Limits","MaxLinearVelocity",Physics::kDef_MaxLinearVel);
    mMaxAngVel = GetConfigValue("Limits","MaxAngularVelocity",Physics::kDef_MaxAngularVel);
    mDragCoeff = GetConfigValue("Limits","DragCoeff",Physics::kDef_DragCoeff);
    mMaxAngMom = GetConfigValue("Limits","MaxAngularMomentum",Physics::kDef_MaxAngularMom);
	mAuxEngine = new AuxEngine();
}
/*!
 @return 
*//*__________________________________________________________________________*/
Physics::Engine::~Engine()
{
	delete mAuxEngine;
	mAuxEngine = 0;
}
/*!
 @param value 
 @return 
*//*__________________________________________________________________________*/
void Physics::Engine::SetGravity(Vector3D value)
{
	mAuxEngine->mGravity = value;
}
/*!
 @param *cb 
*//*__________________________________________________________________________*/
void Physics::Engine::AddPhysicsCallback(int type, PhysicsCB callbackFn)
{
	if(callbackFn)
		mAuxEngine->mCallbacks[type] = callbackFn;
}
/*!
 @param radius 
 @return 
*//*__________________________________________________________________________*/
uint32_t Physics::Engine::AddRigidBodySphere(Real radius)
{
	uint32_t id					= UniqueID();
	RigidBody* body				= new RigidBody();
	GeometryType* collide		= 0;
	collide		= new Sphere(radius);
	mAuxEngine->mBodies[id]	= body;

	body->InertiaKind(kI_Sphere);
	body->SetCollisionObject(collide);
	body->AngVelocityDamp(200.0);
	return id;
}
/*!
 @param plane 
 @return 
*//*__________________________________________________________________________*/
uint32_t Physics::Engine::AddRigidBodyPlane(Geometry::Plane3D& plane)
{
	uint32_t id = UniqueID();
	RigidBody* body				= new RigidBody();
	GeometryType* collide		= 0;
	collide		= new Plane(plane);
	
	mAuxEngine->mBodies[id]	= body;

	body->InertiaKind(kI_Immobile);
	body->SetCollisionObject(collide);
	body->Spinnable(false);
	body->Translatable(false);

	return id;
}
/*!
 @param p0 A point on the plane.
 @param p1 A point on the plane.
 @param p2 A point on the plane.
 @return A unique id in the physics simulation.
*//*__________________________________________________________________________*/
uint32_t Physics::Engine::AddRigidBodyBoundedPlane(const Physics::BoundedPlane& p)
{
	uint32_t id = UniqueID();
	RigidBody* body				= new RigidBody();
	GeometryType* collide		= 0;

	collide		= new BoundedPlane(p);

	mAuxEngine->mBodies[id]	= body;

	body->InertiaKind(kI_Immobile);
	body->SetCollisionObject(collide);
	body->Spinnable(false);
	body->Translatable(false);

	return id;
}
/*!
 @param id 
 @return 
*//*__________________________________________________________________________*/
bool Physics::Engine::RemoveRigidBody(uint32_t id)
{
	bool ret = false;

	if(mAuxEngine->mBodies.count(id) != 0)
	{
		// remove any springs that may be attached to body being removed.
		SpringMap::iterator it = mAuxEngine->mSprings.begin();
		for(; it != mAuxEngine->mSprings.end(); ++it)
		{
			Spring* spring = it->second;
			if(spring->mBody1 == id || spring->mBody2 == id)
			{
				RemoveSpring(it->first);
				it = mAuxEngine->mSprings.begin();
			}
		}
		RigidBody* body = mAuxEngine->mBodies[id];
		mAuxEngine->mBodies.erase(id);
		delete body;
		ret = true;
	}

	return ret;
}
/*!
 @param void 
*//*__________________________________________________________________________*/
void Physics::Engine::RemoveAll(void)
{
	RigidBodyMap::iterator	bIt;
	SpringMap::iterator		sIt;

	for(bIt = mAuxEngine->mBodies.begin(); bIt != mAuxEngine->mBodies.end(); ++bIt)
	{
		RigidBody* body = bIt->second;
		delete body;
	}
	for(sIt = mAuxEngine->mSprings.begin(); sIt != mAuxEngine->mSprings.end(); ++sIt)
	{
		Spring* spring = sIt->second;
		delete spring;
	}
	mAuxEngine->mBodies.clear();
	mAuxEngine->mSprings.clear();
}
/*!
 @param void 
 @return 
*//*__________________________________________________________________________*/
uint32_t Physics::Engine::AddSpring(void)
{
	uint32_t id					= UniqueID();
	Spring* spring				= new Spring();
	mAuxEngine->mSprings[id]	= spring;

	return id;
}
/*!
 @param uint32_t 
 @return 
*//*__________________________________________________________________________*/
bool Physics::Engine::RemoveSpring(uint32_t id)
{
	bool ret = false;
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		mAuxEngine->mSprings.erase(id);
		delete spring;
		ret = true;
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::RigidBodyBool(uint32_t id, eRigidBodyBool prop, bool value)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propActive:
			body->Active(value);		break;
		case propUseGravity:
			body->Gravity(value);		break;
		case propCollidable:
			body->Collidable(value);	break;
		case propSpinnable:
			body->Spinnable(value);		break;
		case propTranslatable:
			body->Translatable(value);	break;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
bool Physics::Engine::RigidBodyBool(uint32_t id, eRigidBodyBool prop)
{
	bool ret = false;
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propActive:
			ret = body->Active();		break;
		case propUseGravity:
			ret = body->Gravity();		break;
		case propCollidable:
			ret = body->Collidable();	break;
		case propSpinnable:
			ret = body->Spinnable();	break;
		case propTranslatable:
			ret = body->Translatable();	break;
		}
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::RigidBodyScalar(uint32_t id, eRigidBodyScalar prop, Real value)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propAngVelDamp:
			body->AngVelocityDamp(value);		break;
		case propLinVelDamp:
			body->LinVelocityDamp(value);		break;
		case propMass:
			body->Mass(value);					break; 
			// mass must be greater than .2 for accuracy
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
Real Physics::Engine::RigidBodyScalar(uint32_t id, eRigidBodyScalar prop)
{
	Real ret = k0;
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propAngVelDamp:
			ret = body->AngVelocityDamp();		break;
		case propLinVelDamp:
			ret = body->LinVelocityDamp();		break;
		case propMass:
			ret = body->Mass();					break; 
		}
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::RigidBodyVector3D(uint32_t id, eRigidBodyVector prop, Vector3D value)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propDimensions:
			body->mExtent = value;				break;
		case propPosition:
			body->mStateT1.mPosition = value;	break;
		case propVeloctity:
			body->mStateT1.mVelocity = value;	break;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
Vector3D Physics::Engine::RigidBodyVector3D(uint32_t id, eRigidBodyVector prop)
{
	Vector3D ret = vZero;
	
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propDimensions:
			ret = body->mExtent;				break;
		case propPosition:
			ret = body->mStateT1.mPosition;		break;
		case propVeloctity:
			ret = body->mStateT1.mVelocity ;	break;
		}
	}

	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::RigidBodyQuaternion(uint32_t id, eRigidBodyQuaternion prop, Quaternion value)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propOrientation:
			body->mStateT1.mOrientation = value;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
Quaternion Physics::Engine::RigidBodyQuaternion(uint32_t id, eRigidBodyQuaternion prop)
{
	Quaternion ret = qZero;
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		switch(prop)
		{
		case propOrientation:
			ret = body->mStateT1.mOrientation;
		}
	}
	return ret;
}
/*!
 @param id 
 @return 
*//*__________________________________________________________________________*/
Matrix4x4 Physics::Engine::RigidBodyTransformationMatrix(uint32_t id)
{
	Matrix4x4 ret;
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		if(body->Spinnable())
		{
			ret = body->mStateT1.mOrientation.Basis();
		}
		else
		{
			ret.reset();
		}
		ret[0][3] = body->mStateT1.mPosition[0];
		ret[1][3] = body->mStateT1.mPosition[1];
		ret[2][3] = body->mStateT1.mPosition[2];
		
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::SpringBool(uint32_t id, eSpringBool prop, bool value)
{
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propSpringCompressible:
			spring->mCompressible = value;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
bool Physics::Engine::SpringBool(uint32_t id, eSpringBool prop)
{
	bool ret = false;
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propSpringCompressible:
			ret = spring->mCompressible;	break;
		}
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::SpringBody(uint32_t id, eSpringID prop, uint32_t value)
{
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propBody1:
			if(mAuxEngine->mBodies.count(value) != 0)
			{
				spring->mBody1 = value;
				spring->mBodyPtr1 = mAuxEngine->mBodies[value];
			}
			else
			{
				char buf[100] = {0};
				sprintf(buf, "Can't attach spring to nonexistant body. Spring: %d, Body: %d", id, value); 
			}
			break;
		case propBody2:
			if(mAuxEngine->mBodies.count(value) != 0)
			{
				spring->mBody2 = value;
				spring->mBodyPtr2 = mAuxEngine->mBodies[value];
			}
			else
			{
				char buf[100] = {0};
				sprintf(buf, "Can't attach spring to nonexistant body. Spring: %d, Body: %d", id, value); 
			}
			break;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
uint32_t Physics::Engine::SpringBody(uint32_t id, eSpringID prop)
{
	uint32_t ret = 0;
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propBody1:
			ret = spring->mBody1; break;
		case propBody2:
			ret = spring->mBody2; break;
		}
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::SpringScalar(uint32_t id, eSpringScalar prop, Real value)
{
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propSpringStiffness:
			spring->mStiffness = value;		break;
		case propSpringRestLength:
			spring->mRestLength = value;
			spring->mPrevLength = value;	break;
		case propSpringDamping:
			spring->mDamping = value;
			if(value > k1 || value < k0)
			{
				char buf[100] = {0};
				sprintf(buf, "Illegal damping value: %f.  0 for no damping, 1 for critical damping.", value);
			}
			break;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
Real Physics::Engine::SpringScalar(uint32_t id, eSpringScalar prop)
{
	Real ret = k0;
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propSpringStiffness:
			ret = spring->mStiffness;		break;
		case propSpringRestLength:
			ret = spring->mRestLength;		break;
		case propSpringDamping:
			ret = spring->mDamping;			break;
		}
	}
	return ret;
}
/*!
 @param id 
 @param prop 
 @param value 
*//*__________________________________________________________________________*/
void Physics::Engine::SpringVector(uint32_t id, eSpringVector prop, Vector3D value)
{
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propAttachPoint1:
			spring->mPos1 = value;
			spring->mCenterAttach1 = (value * value) <= kEpsilon ? true : false; // true if zero vector
			break;
		case propAttachPoint2:
			spring->mPos2 = value;
			spring->mCenterAttach2 = (value * value) <= kEpsilon ? true : false; // true if zero vector
			break;
		}
	}
}
/*!
 @param id 
 @param prop 
 @return 
*//*__________________________________________________________________________*/
Vector3D Physics::Engine::SpringVector(uint32_t id, eSpringVector prop)
{
	Vector3D ret = vZero;
	if(mAuxEngine->mSprings.count(id) != 0)
	{
		Spring* spring = mAuxEngine->mSprings[id];
		switch(prop)
		{
		case propAttachPoint1:
			ret = spring->mPos1;	break;
		case propAttachPoint2:
			ret = spring->mPos2;	break;
		}
	}
	return ret;
}
/*!
 @param id 
 @param force 
*//*__________________________________________________________________________*/
void Physics::Engine::AddImpulse(uint32_t id, Vector3D force)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		if(body->Translatable())
			body->mAccum.AddForce(force);
	}
}
/*!
 @param id 
 @param torque 
*//*__________________________________________________________________________*/
void Physics::Engine::AddTwist(uint32_t id, Vector3D torque)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		if(body->Spinnable())
			body->mAccum.AddTorque(torque);
	}
}
/*!
 @param id 
*//*__________________________________________________________________________*/
void Physics::Engine::StopMoving(uint32_t id)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		if(body->Translatable())
		{
			body->mAccum.mForce = vZero;
			body->mStateT0.mVelocity = vZero;
			body->mStateT1.mVelocity = vZero;
		}
	}
}
/*!
 @param id 
*//*__________________________________________________________________________*/
void Physics::Engine::StopSpinning(uint32_t id)
{
	if(mAuxEngine->mBodies.count(id) != 0)
	{
		RigidBody* body = mAuxEngine->mBodies[id];
		//if(body->Spinnable())
		{
			body->mAccum.mTorque = vZero;
			body->mStateT0.mAngularVelocity = vZero;
			body->mStateT1.mAngularVelocity = vZero;
			body->mStateT0.mAngularMomentum = vZero;
			body->mStateT1.mAngularMomentum = vZero;
		}
	}
}
/*!
 @param dt 
*//*__________________________________________________________________________*/
void Physics::Engine::SetMinTimeStep(Real dt)
{
	mAuxEngine->mMinTimeStep = dt;
}
/*!
 @param dt 
*//*__________________________________________________________________________*/
void Physics::Engine::Update(Real dt, int steps)
{
//ProfileFn;
   // LogS->Post(__FUNCTION__);
   // static int call_count = 0;
    if(AtRest())
            return;
    
    mIsStatic = true;
    //steps *= 3;
		// fluid drag constant
		//const Real b = 50.0;
		//RigidBodyMap::iterator bIt = mAuxEngine->mBodies.begin();
		//volatile int staticLinearCount = 0;
		//volatile int staticAngularCount = 0;
		volatile int activeCount = 0;
		//volatile int SpinnableCount = 0;
//		for(; bIt != mAuxEngine->mBodies.end(); ++bIt)

        std::vector< Ball * > &balls = Game::Get()->GetPlayfield()->mBalls;
        
        for(unsigned int i = 0; i < balls.size(); ++i)
		{
            int id = balls[i]->ID();
            
            Vector3D v = RigidBodyVector3D(id, Physics::Engine::eRigidBodyVector::propVeloctity);
            if(v.length() > .2)
            {
                Physics::Engine::AddImpulse(id, (-mDragCoeff/steps) * v);
				++activeCount;
                mIsStatic = false;
            }
            else if(v.length() > kEpsilon)
			{
					StopMoving(id);
            }
        }
		

		if(activeCount)
		{
            for(int i = 0; i < steps; ++i)
	    	{
			    Simulate(dt/steps);

    		}
			
	    }
        else if(!mWasStatic)
        {
           mAuxEngine->mCallbacks[kCallbackGoneStatic](0,0,0);
        }	
		
		
		mWasStatic = mIsStatic;
}
/*!
 @param void 
 @return 
*//*__________________________________________________________________________*/
bool Physics::Engine::AtRest(void)const { return mIsStatic; }
void Physics::Engine::AtRest(bool b) { mIsStatic = b; }
/*!
 @param void 
*//*__________________________________________________________________________*/
void Physics::Engine::StopAll(void)
{
	RigidBodyMap::iterator bIt = mAuxEngine->mBodies.begin();
	for(; bIt != mAuxEngine->mBodies.end(); ++bIt)
	{
		StopMoving(bIt->first);
		StopSpinning(bIt->first);
	}
}
uint32_t	Physics::Engine::Kind(uint32_t id)const
{
	return (mAuxEngine->mBodies[id])->mCollideGeom->Kind();
}
/*!
 @param dt 
*//*__________________________________________________________________________*/
void Physics::Engine::Simulate(Real dt)
{	ProfileFn;
	//LogS->Post(__FUNCTION__);
	int steps;
	if(dt > mAuxEngine->mMinTimeStep)
	{
		steps = 1 + (int)(dt / mAuxEngine->mMinTimeStep);
		dt /= (float)steps;
	}
	else
	{
		steps = 1;
	}

	// reset for current timestep
	for(int i = 0; i < steps; ++i)
	{
		RigidBodyMap::iterator	bIt;
		SpringMap::iterator		sIt;

		for(bIt = mAuxEngine->mBodies.begin(); bIt != mAuxEngine->mBodies.end(); ++bIt)
		{
			RigidBody *body = bIt->second;
			if(body->Active())
			{
				body->ResetForNextTimeStep();
			}
		}
	

		// loop over all objects
		for(bIt = mAuxEngine->mBodies.begin(); bIt != mAuxEngine->mBodies.end(); ++bIt)
		{
			RigidBody *body = bIt->second;
			if(body->Active())
				body->Integrate1(dt);
		}

		// loop over all springs and apply forces to objects
		for(sIt = mAuxEngine->mSprings.begin(); sIt != mAuxEngine->mSprings.end(); ++sIt)
		{
			Spring* spring	= sIt->second;
			Vector3D Pos1 = spring->mBodyPtr1->mStateT1.mPosition;
			Vector3D Pos2 = spring->mBodyPtr2->mStateT1.mPosition;
			Vector3D dir = Pos1 - Pos2;
			Real length = (Real)(dir.length());
			Real x = length - spring->mRestLength;
			if(spring->mCompressible || (x > k0))
			{
				Real force = spring->mStiffness * x;
				Real v = spring->mPrevLength - x;
				force += v * spring->mDamping;
				spring->mPrevLength = length;
				spring->mBodyPtr1->mAccum.mForce += -force * dir;
				spring->mBodyPtr2->mAccum.mForce += force * dir;
			}
		
		}
		// loop over all objects again after spring forces
		for(bIt = mAuxEngine->mBodies.begin(); bIt != mAuxEngine->mBodies.end(); ++bIt)
		{
			RigidBody* body	= bIt->second;
			if(body->Active())
				body->Integrate2(dt, mAuxEngine->mGravity);
		}

		// loop over all objects, detect and resolve collisions
		mAuxEngine->mCollisionEngine.Begin();

        for(bIt = mAuxEngine->mBodies.begin(); bIt != mAuxEngine->mBodies.end(); ++bIt)
		{
			RigidBody* body1 = bIt->second;
            // create a vector by which to displace all of the other balls, to simplify the 
            // reference frame
            Geometry::Vector3D disp = -body1->mStateT0.mVelocity;
            // this will be the new location of the balls, if it is a ball at all :)
            Geometry::Vector3D pos1;
            Geometry::Vector3D pos2;
            Geometry::Vector3D vel;

			if(body1->Collidable())
			{
                // start looking through the list from where we are, not from the beginning

				RigidBodyMap::iterator bIt2 = bIt;
				++bIt2;
				for(; bIt2 != mAuxEngine->mBodies.end(); ++bIt2)
				{
					RigidBody* body2 = bIt2->second;
                    // test that the body can be collided with
                    if(!body2->Collidable())
                        continue;
                    
                    // if the objects are not moving, don't test the collision
                    if(body1->mStateT0.mVelocity.length() < kEpsilon && body2->mStateT0.mVelocity.length() < kEpsilon)
                        continue;

                    // minimize sphere collision tests
                    if(body1->mCollideGeom->Kind() == Physics::kC_Sphere && body2->mCollideGeom->Kind() == Physics::kC_Sphere)
                    {
                        // the position of body 1 at the beginning of the frame
                        pos1 = body1->mStateT0.mPosition;
                        // the velocity of body2 in body1 reference frame
                        vel = body2->mStateT0.mVelocity + disp;
                        // the position of body2 in body1 reference frame
                        pos2 = body2->mStateT0.mPosition + vel * dt;
                        // check body distance for each axis, bail out if too far
                        if(abs(pos1[2] - pos2[2]) >= 3.f)
                            continue;
                        if(abs(pos1[0] - pos2[0]) >= 3.f)
                            continue;
                        if(abs(pos1[1] - pos2[1]) >= 3.f)
                            continue;
                    }
                    // passed the tests, check for collision
					mAuxEngine->mCollisionEngine.TestCollision(body1, body2);
				}
			}
		}
        
  
        std::sort(mAuxEngine->mCollisionEngine.mContacts.begin(), mAuxEngine->mCollisionEngine.mContacts.end(), CollisionSortPred());

		std::vector< Collision::Contact* >::iterator cIt;
        
		// get the player, and populate the contact list

        for(cIt = mAuxEngine->mCollisionEngine.mContacts.begin(); cIt != mAuxEngine->mCollisionEngine.mContacts.end(); ++cIt)
		{
			// resolve the collisions for this iteration
			mAuxEngine->mCollisionEngine.Resolve(*cIt);
		}
    	
		// clean up collision free list
		mAuxEngine->mCollisionEngine.End();

		for(bIt = mAuxEngine->mBodies.begin(); bIt != mAuxEngine->mBodies.end(); ++bIt)
		{
			RigidBody* body = bIt->second;
			body->Renormalize();
		}
       
      
	}




}

