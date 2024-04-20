/*!
	@file	CollisionEngine.cpp
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"

#include "CollisionEngine.h"
#include "log.h"
#include "RuleSystem.h"
#include "RigidBody.h"
#include "PhysicsAux.h"
#include "profiler.h"
#include "game.h"

using Physics::RigidBody;
using Geometry::Plane3D;
using Physics::AuxEngine;

namespace Collision
{
	static inline bool Equal(Vector3D u, Vector3D v)
	{
		return (Math::Abs((Real)u[0] - (Real)v[0]) + Math::Abs((Real)u[1] - (Real)v[1]) + Math::Abs((Real)u[2] - (Real)v[2])) < kEpsilon;
	}
	
	typedef	bool (*CollisionFn)(AuxEngine*, Contact*, RigidBody* body1, RigidBody* body2);
	typedef void (*ResolutionFn)(Contact*);

    /*!
     @param contact 
     @param body1 
     @param body2 
     @return 
    *//*__________________________________________________________________________*/
    bool CollidePlanePlane(AuxEngine* Aux, Contact* contact, RigidBody* plane1, RigidBody* plane2)
	{
        //ProfileFn;
		Aux;		// shut up compiler - parameter present to satisfy signature for fp.
		contact;	// shut up compiler - parameter present to satisfy signature for fp.
		bool collided = false;
		if(!(plane1->Translatable() & plane1->Spinnable() & plane2->Translatable() & plane2->Spinnable()))
		{
			collided = !Equal(plane1->mExtent, plane2->mExtent);
		}
		else
			collided = true;
		return collided;
	}
	/*!
	 @param contact 
	 @param plane 
	 @param sphere 
	 @return 
	*//*__________________________________________________________________________*/
	bool CollidePlaneSphere(AuxEngine* Aux, Contact* contact, RigidBody* plane, RigidBody* sphere)
	{
        //ProfileFn;
		Plane3D p = ((Physics::Plane*)(plane->mCollideGeom))->mPlane;
		Vector3D	v0 = sphere->mStateT0.mPosition,
					v1 = sphere->mStateT1.mPosition;	
		Geometry::Point3D	c0(v0[0], v0[1], v0[2]),
							c1(v1[0], v1[1], v1[2]);
		Real d0 = (Real)Geometry::Distance(c0, p);
		Real d1 = (Real)Geometry::Distance(c1, p);
		Real radius = ((Physics::Sphere*)sphere->mCollideGeom)->mRadius;
		bool ret = false;
		
		

		if(Math::Abs(d0) <= radius + kEpsilon)
		{
			contact->mNormal = p.normal();
			contact->mTime = k0;
			ret = true;
		}
		else if(d0 > radius && d1 < radius)
		{
			// penetration
			contact->mNormal = p.normal();
			Real u = (d0 - radius)/(d0 - d1);
			v1 *= u;
			v0 *= (k1 - u);
			contact->mPosition = v0 + v1;
			ret = true;
			contact->mTime = u;
		}
		
		// check to see if the plane is bounded, then perform a containment check for the contact point.
		if(ret && plane->mCollideGeom->Kind() == Physics::kC_BoundedPlane)
		{
			Geometry::Point3D pt(contact->mPosition[0], contact->mPosition[1], contact->mPosition[2]);
			if(!((Physics::BoundedPlane*)(plane->mCollideGeom))->mPolygon.contains(pt))
			{
				ret = false;
			}
			else
			{	
				Physics::RigidBodyMap::iterator it = Aux->mBodies.begin();
				for(;it != Aux->mBodies.end(); ++it)
				{
					if(plane == it->second)
					{
						contact->mID1 = it->first;
						contact->mBody1 = it->second;
						Physics::RigidBodyMap::iterator it2 = Aux->mBodies.begin();
						for(;it2 != Aux->mBodies.end(); ++it2)
						{
							if(sphere == it2->second)
							{
								contact->mID2 = it2->first;
								contact->mBody2 = it2->second;
							}
						}
					}
				}
				if(Aux->mCallbacks.count(kCollisionCBSpherePocket) != 0 && Aux->mCallbacks[kCollisionCBSpherePocket] != 0)
				{
					Aux->mCallbacks[kCollisionCBSpherePocket](contact, plane, sphere);
				}
				if(ret && Aux->mCallbacks.count(kCallbackRuleSBP) != 0 && Aux->mCallbacks[kCallbackRuleSBP] != 0)
				{
					//Aux->mCallbacks[kCallbackRuleLog](contact, plane, sphere);
					Aux->mCallbacks[kCallbackRuleSBP](contact, plane, sphere);
				}
				return true;
			}
		}

		// (jmp) Send a message.
		if(ret && Aux->mCallbacks.count(kCollisionCBSpherePlane) != 0 && Aux->mCallbacks[kCollisionCBSpherePlane] != 0)
		{
			Physics::RigidBodyMap::iterator it = Aux->mBodies.begin();
			for(;it != Aux->mBodies.end(); ++it)
			{
				if(plane == it->second)
				{
					contact->mID1 = it->first;
					contact->mBody1 = it->second;
					Physics::RigidBodyMap::iterator it2 = Aux->mBodies.begin();
					for(;it2 != Aux->mBodies.end(); ++it2)
					{
						if(sphere == it2->second)
						{
							contact->mID2 = it2->first;
							contact->mBody2 = it2->second;
						}
					}
				}
			}
			Aux->mCallbacks[kCollisionCBSpherePlane](contact, plane, sphere);
		}
		if(ret && Aux->mCallbacks.count(kCallbackRuleSP) != 0 && Aux->mCallbacks[kCallbackRuleSP] != 0)
		{
			//Aux->mCallbacks[kCallbackRuleLog](contact, plane, sphere);
			Aux->mCallbacks[kCallbackRuleSP](contact, plane, sphere);
		}
		return ret;
	}
	/*!
	 @param contact 
	 @param sphere 
	 @param plane 
	 @return 
	*//*__________________________________________________________________________*/
	bool CollideSpherePlane(AuxEngine* Aux, Contact* contact, RigidBody* sphere, RigidBody* plane)
	{
		return CollidePlaneSphere(Aux, contact, plane, sphere);
	}

	inline bool QuadraticFormula(Real a, Real b, Real c, Real& root1, Real& root2)
	{
        //ProfileFn;
		Real q = b*b - 4.f * a * c;
		if(q >= k0)
		{
			Real sq = /*Math::S*/sqrt(q);
			Real d = k1 / (2.f * a);
			root1 = (-b + sq) * d;
			root2 = (-b - sq) * d;
			return true;
		}
		else
			return false; // complex roots
	}

	/*!
	 @param contact 
	 @param sphere1 
	 @param sphere2 
	 @return 
	*//*__________________________________________________________________________*/
	bool CollideSphereSphere(AuxEngine* Aux, Contact* contact, RigidBody* sphere1, RigidBody* sphere2)
	{
        ProfileFn;
		//Initialize the return value
		bool ret = false;
        float tmin = kMax;

        //double tmin = 0.0f;

		Real	radius1 = ((Physics::Sphere*)sphere1->mCollideGeom)->mRadius, 
				radius2 = ((Physics::Sphere*)sphere2->mCollideGeom)->mRadius;

		// Relative velocity
		Vector3D va = sphere1->mStateT1.mPosition - sphere1->mStateT0.mPosition;
		Vector3D vb = sphere2->mStateT1.mPosition - sphere2->mStateT0.mPosition;
		Vector3D    dv = vb - va;//obj2->prVelocity - obj1->prVelocity;

		
		// Relative position
		Vector3D	dp = sphere2->mStateT0.mPosition - sphere1->mStateT0.mPosition;//obj2->prPosition - obj1->prPosition;
       		
		//Minimal distance squared
		float r = radius1 + radius2;//obj1->fRadius + obj2->fRadius;
		//dP^2-r^2
		float pp = dp[0] * dp[0] + dp[1] * dp[1] + dp[2] * dp[2] - r * r;
		//(1)Check if the spheres are already intersecting
		if ( pp < 0 ) 
		{
			contact->mNormal = (sphere1->mStateT1.mPosition - sphere2->mStateT1.mPosition).normal();
			ret = true;
 //           goto exit_ss;
		}

		//dP*dV
		float pv    = dp[0] * dv[0] + dp[1] * dv[1] + dp[2] * dv[2];
		//(2)Check if the spheres are moving away from each other
		if ( pv >= 0 ) 
        {
			ret = false;
            goto exit_ss; 
        }

		//dV^2
		float vv = dv[0] * dv[0] + dv[1] * dv[1] + dv[2] * dv[2];
		//(3)Check if the spheres can intersect within 1 frame
		if ( (pv + vv) <= 0 && (vv + 2 * pv + pp) >= 0 ) 
        {
			ret = false;
            goto exit_ss;
        }

		//tmin = -dP*dV/dV*2 
		//the time when the distance between the spheres is minimal
		tmin = -pv/vv;

		//Discriminant/(4*dV^2) = -(dp^2-r^2+dP*dV*tmin)
		//ret = ( pp + pv * tmin > 0 );
		
		//if(ret)
		//{
		//	contact->mNormal = (sphere1->mStateT1.mPosition - sphere2->mStateT1.mPosition).normal();
		//	contact->mPosition = (sphere1->mStateT0.mPosition + tmin * (sphere1->mStateT0.mVelocity.normal())) + radius1 * contact->mNormal;
			
	//	}
		//return ret;
	//	Aux;  // shut up the compiler
	//	
/****************************
This one ~works
****************************/
		//bool ret = false;
  //      Real	u0, u1;
		//Real	radius1 = ((Physics::Sphere*)sphere1->mCollideGeom)->mRadius, 
		//		radius2 = ((Physics::Sphere*)sphere2->mCollideGeom)->mRadius;
		//Vector3D va = sphere1->mStateT1.mPosition - sphere1->mStateT0.mPosition;
		//Vector3D vb = sphere2->mStateT1.mPosition - sphere2->mStateT0.mPosition;
		//Vector3D vab = vb - va;
		//Vector3D ab = sphere2->mStateT0.mPosition - sphere1->mStateT0.mPosition;
		//Real	rab = radius1 + radius2;
		//Real	a = (Real)(vab * vab);
		//Real	b = 2.f * (Real)(vab * ab);
		//Real	c = (Real)(ab * ab) - Math::Sqr(rab);

		//if(c <= k0)
		//{
		//	u0 = k0;
		//	u1 = k0;
		//	ret = true;
		//}
		//if(!ret)
		//{
		//	// test the relative velocities
		//	if(Math::Abs(a) < kEpsilon)
		//		ret = false;			
		//}
		//else if(QuadraticFormula(a, b, c, u0, u1))
		//{
		//	if(u0 > u1)
		//		Math::Swap(u0, u1);
		//	if(u0 < k0 &&  u1 < k0)
		//		ret = false;
		//	else if(u1 > k1 && u0 > k1)
		//		ret = false;
		//	else
		//		ret = true;
		//}
exit_ss:
		// employ callback mechanism
		if(ret)
		{
			contact->mNormal = (sphere1->mStateT1.mPosition - sphere2->mStateT1.mPosition).normal();
            //contact->mNormal = ((sphere2->mStateT0.mPosition + (tmin * sphere2->mStateT0.mVelocity.normal())) - (sphere1->mStateT0.mPosition + (tmin * sphere1->mStateT0.mVelocity.normal()))).normal();
			contact->mBody1 = sphere1;
			contact->mBody2 = sphere2;
            contact->mTime = tmin;
            contact->mPosition = (sphere1->mStateT0.mPosition + tmin * (sphere1->mStateT0.mVelocity.normal())) + radius1 * contact->mNormal;
			Physics::RigidBodyMap::iterator it = Aux->mBodies.begin();
			while(it != Aux->mBodies.end())
			{
				if(it->second == sphere1)
				{
					contact->mID1 = it->first;
					contact->mBody1 = sphere1;
				}
				else if(it->second == sphere2)
				{
					contact->mID2 = it->first;
					contact->mBody2 = sphere2;
				}
				++it;
			}
		}
		if(ret && Aux->mCallbacks.count(kCallbackRuleSS) != 0 && Aux->mCallbacks[kCallbackRuleSS] != 0)
		{
			//Aux->mCallbacks[kCallbackRuleLog](contact, plane, sphere);
			Aux->mCallbacks[kCallbackRuleSS](contact, sphere1, sphere2);
		}
		if(ret && Aux->mCallbacks.count(kCollisionCBSphereSphere) != 0 && Aux->mCallbacks[kCollisionCBSphereSphere] != 0)
		{
			//Aux->mCallbacks[kCallbackRuleLog](contact, plane, sphere);
			Aux->mCallbacks[kCollisionCBSphereSphere](contact, sphere1, sphere2);
		}

		
		return ret;
	}

	//CollisionFn CollisionFunctions[2][2] = {	CollidePlanePlane, CollidePlaneSphere, CollideSpherePlane, CollideSphereSphere };
	CollisionFn CollisionFunctions[3][3] = {	CollidePlanePlane, CollidePlaneSphere, CollidePlanePlane,
												CollideSpherePlane, CollideSphereSphere, CollideSpherePlane,
												CollidePlanePlane, CollidePlaneSphere, CollidePlanePlane};

	/*!
	 @param body1 
	 @param body2 
	 @return 
	*//*__________________________________________________________________________*/
	Contact* Collision::Engine::TestCollision(RigidBody* body1, RigidBody* body2)
	{
        //ProfileFn;
		Contact* ret = 0;

    ENFORCE(mFreePool.size() != 0)("Contact pool is empty."); // (jmp)
		/*if(mFreePool.size() == 0)
		{ 
			throw(nsl::exception("Contact pool is empty.", __FILE__, __LINE__, __FUNCTION__));
			exit(0);
		}*/
		Contact* contact = mFreePool.front();
		mFreePool.pop_front();

		if(CollisionFunctions[body1->mCollideGeom->Kind()][body2->mCollideGeom->Kind()](mParentPE, contact, body1, body2))
		{
			mContacts.push_back(contact);
			contact->mBody1 = body1;
			contact->mBody2 = body2;
			ret = contact;
		}
		else
		{
			mFreePool.push_back(contact);
		}

        return ret;
	}

	/*!
	 @param contact 
	*//*__________________________________________________________________________*/
	void _ResolveSpherePlane(Contact* contact)
	{
        //ProfileFn;
		//RigidBody* plane = contact->mBody1;
		RigidBody* sphere = contact->mBody2;
		Vector3D temp;
		Vector3D vContact;
		Physics::Sphere* spherePtr = (Physics::Sphere*)sphere->mCollideGeom;
		vContact = -spherePtr->mRadius * contact->mNormal;
		Vector3D vel1 = sphere->mStateT1.mVelocity;

		if(sphere->Spinnable())
		{
			vel1 += (sphere->mStateT1.mAngularVelocity ^ vContact);
		}

		Real velNormalDir = (Real)(vel1 * contact->mNormal);
		if(velNormalDir < k0)
		{
			Real ImpulseNumerator = -(k1 + CoeffRestSP) * velNormalDir;
			temp = vContact ^ vel1;//contact->mNormal;
			if(sphere->InertiaKind() == Physics::kI_Sphere)
			{
				temp *= sphere->mInertiaTensorInv[0];
			}
			else
			{
				temp[0] = temp[0] * sphere->mInertiaTensorInv[0];
				temp[1] = temp[1] * sphere->mInertiaTensorInv[1];
				temp[2] = temp[2] * sphere->mInertiaTensorInv[2];
			}
			Vector3D temp2 = temp ^ vContact;

			Real ImpulseDenominator = (Real)(sphere->MassInv() + (temp2 * contact->mNormal));
			Real result = (sphere->MassInv() * ImpulseNumerator) / ImpulseDenominator;
			Vector3D impulse = result * contact->mNormal;
			sphere->mStateT1.mVelocity += impulse;
			//sphere->mStateT1.mAngularMomentum += .25 * temp2;//contact->mNormal ^ impulse;
			sphere->mStateT1.mAngularVelocity += temp2;// ^ impulse;
		}
	}
	/*!
	 @param contact 
	*//*__________________________________________________________________________*/
	void ResolvePlanePlane(Contact* contact)
	{
		contact;	// shut up compiler - parameter present to satisfy signature for fp.
	}
	/*!
	 @param contact 
	*//*__________________________________________________________________________*/
	void ResolvePlaneSphere(Contact* contact)
	{
        //ProfileFn;
		//RigidBody* plane = contact->mBody1;
		RigidBody* sphere = contact->mBody2;
		Vector3D adjust = sphere->mStateT1.mPosition - contact->mPosition;
		Real residual = (Real)adjust.length();
		sphere->mStateT1.mPosition = contact->mPosition;

		_ResolveSpherePlane(contact);

		adjust = sphere->mStateT1.mVelocity.normal();
		sphere->mStateT1.mPosition += residual * adjust;
	}
	/*!
	 @param contact 
	*//*__________________________________________________________________________*/
	void ResolveSpherePlane(Contact* contact)
	{
        //ProfileFn;
		RigidBody* temp = contact->mBody1;
		contact->mBody1 = contact->mBody2;
		contact->mBody2 = temp;
		contact->mNormal *= kN1;
		ResolvePlaneSphere(contact);
	}
	/*!
	 @param contact 
	*//*__________________________________________________________________________*/
	void ResolveSphereSphere(Contact* contact)
	{
        //ProfileFn;
		RigidBody* body1 = contact->mBody1;
		RigidBody* body2 = contact->mBody2;
		Vector3D temp, temp2;
		Vector3D contact1, contact2;
		Vector3D velocityA, velocityB, velocityAB;

		Physics::Sphere* sphere1 = (Physics::Sphere*)body1->mCollideGeom;
		Physics::Sphere* sphere2 = (Physics::Sphere*)body2->mCollideGeom;

		contact1 = -sphere1->mRadius * contact->mNormal;
		contact2 = sphere2->mRadius * contact->mNormal;
		velocityA = body1->mStateT1.mVelocity;
		velocityB = body2->mStateT1.mVelocity;

		if(body1->Spinnable())
		{
			temp = body1->mStateT0.mAngularVelocity ^ contact1;
			velocityA += temp;
		}
		if(body2->Spinnable())
		{
			temp = body2->mStateT0.mAngularVelocity ^ contact2;
			velocityB += temp;
		}
		velocityAB = velocityA - velocityB; // relative velocity
		Real velNormalDir = (Real)(velocityAB * contact->mNormal);

		if(velNormalDir < k0) // objects are moving toward one another
		{
			Real impulseNumerator = -(k1 + CoeffRestSS) * velNormalDir;
			temp = contact1 ^ velocityA;//contact->mNormal;
			if(body1->InertiaKind() == Physics::kI_Sphere)
			{
				temp *= body1->mInertiaTensorInv[0];
			}
			else
			{
				temp[0] = temp[0] * body1->mInertiaTensorInv[0];
				temp[1] = temp[1] * body1->mInertiaTensorInv[1];
				temp[2] = temp[2] * body1->mInertiaTensorInv[2];
			}
			temp2 = temp ^ contact1;
			Real impulseDenominator = (Real)(body1->MassInv() + (temp2 * contact->mNormal) + body2->MassInv());

			if(body2->InertiaKind() == Physics::kI_Sphere)
			{
				temp *= body2->mInertiaTensorInv[0];
			}
			else
			{
				temp[0] = temp[0] * body2->mInertiaTensorInv[0];
				temp[1] = temp[1] * body2->mInertiaTensorInv[1];
				temp[2] = temp[2] * body2->mInertiaTensorInv[2];
			}
			// apply impulse to body1
			temp2 = temp ^ contact2;
			impulseDenominator += Real(temp2 * contact->mNormal);
			Vector3D impulse;
			Real result = (body1->MassInv() * impulseNumerator) / impulseDenominator;
			impulse = result * contact->mNormal;
			body1->mStateT1.mVelocity += impulse;
			// apply opposite impulse to body2
			Vector3D negImpulse = kN1 * body2->MassInv() * impulse;
			body2->mStateT1.mVelocity += negImpulse;

			temp2 += contact->mNormal ^ impulse;
			body1->mStateT1.mAngularMomentum += .05f * temp2;
			temp2 *= kN1;
			body2->mStateT1.mAngularMomentum += .05f * temp2;
		}
	}
	//ResolutionFn ResolveFunctions[2][2] = { ResolvePlanePlane, ResolvePlaneSphere, ResolveSpherePlane, ResolveSphereSphere };
	ResolutionFn ResolveFunctions[3][3] = { ResolvePlanePlane, ResolvePlaneSphere, ResolvePlanePlane,
											ResolveSpherePlane, ResolveSphereSphere, ResolveSpherePlane,
											ResolvePlanePlane, ResolvePlaneSphere, ResolvePlanePlane};
	/*!
	 @param contact 
	*//*__________________________________________________________________________*/
	void Collision::Engine::Resolve(Contact* contact)
	{
		ResolveFunctions[contact->mBody1->mCollideGeom->Kind()][contact->mBody2->mCollideGeom->Kind()](contact);
		contact->mBody1->mCollided = true;
		contact->mBody2->mCollided = true;
	}
	/*!
	 @return 
	*//*__________________________________________________________________________*/
	Collision::Engine::Engine(Physics::AuxEngine* parent):mParentPE(parent)
	{
		SetCapacity(2048);
	}
	/*!
	 @return 
	*//*__________________________________________________________________________*/
	Collision::Engine::~Engine() 
	{ 
        // clean up the collision free pool
		std::list< Collision::Contact* >::iterator it = mFreePool.begin();
        while(it != mFreePool.end())
        {
            delete (*it);
            ++it;
        }
	}
	/*!
	*//*__________________________________________________________________________*/
	void Collision::Engine::Begin() {}
	/*!
	*//*__________________________________________________________________________*/
	void Collision::Engine::End()
	{
		while(mContacts.size() > 0)
		{
			Contact* contact = mContacts.back();
			mContacts.pop_back();
			mFreePool.push_back(contact);
		}
	}
	/*!
	 @param maxContacts 
	*//*__________________________________________________________________________*/
	void Collision::Engine::SetCapacity(int maxContacts)
	{
		int num = maxContacts - (int)mFreePool.size();
		while(--num >= 0)
			mFreePool.push_back(new Contact());
	}

} // namespace Collision