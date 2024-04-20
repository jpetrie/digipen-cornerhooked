/*!
	@file	PhysicsEngine.h
	@author	Scott Smith
	@date	May 04, 2004

	@brief	Internal definitions.
______________________________________________________________________________
 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 */
#pragma once

#ifndef	_PHYSICSENGINE_H_
#define	_PHYSICSENGINE_H_

#include "main.h"

#include "MathDefs.h"
#include "PhysicsDefs.h"
#include "Quaternion.h"
#include "Matrix.hpp"
#include "RigidBody.h"
#include "CollisionEngine.h"

using Geometry::Matrix4x4;

const Real kGravity	= 1.f;
typedef void (*PhysicsCB)(Collision::Contact*, Physics::RigidBody*, Physics::RigidBody* body2);
/*!
	@namespace Physics
	@brief	Contains functions for handling a physics simulation.
*//*__________________________________________________________________________*/
namespace Physics
{
	/*!
	 @class		Engine
	 @ingroup	Physics Engine Proto
	 @date		05-05-2004
	 @author	Scott
	______________________________________________________________________________*/
	class Engine 
	{
	public:
		Engine(void);
		virtual ~Engine();

		uint32_t AddRigidBodyPlane(Geometry::Plane3D& plane);
		uint32_t AddRigidBodySphere(Real radius);
		uint32_t AddRigidBodyBoundedPlane(const Physics::BoundedPlane& p);
		bool RemoveRigidBody(uint32_t id);
		void RemoveAll();

        virtual bool IsAI(void) { return false; }
        		
		/*!
		  @brief Recover a configuration constant value.
		  @param sec  The section in which the value is located.
		  @param key  The value's key.
		  @param def  The default value to use if the key is not found.
		*/ 
		template< typename T_ > T_ GetConfigValue(const std::string &sec,const std::string &key,const T_ def)
		{
		std::string  def_val = lexical_cast< std::string >(def);
		char         res_val[256];
		
		  ::GetPrivateProfileString(sec.c_str(),key.c_str(),def_val.c_str(),res_val,256,"data/config/physics.ini");
		  return (lexical_cast< T_ >(res_val));
		}
		
		/*!
			@enum eRigidBodyBool
			@brief	Boolean properties for rigid bodies.
		*//*__________________________________________________________________________*/
		enum eRigidBodyBool 
		{ 
			propActive = 0, 
			propUseGravity, 
			propCollidable, 
			propSpinnable, 
			propTranslatable 
		};
		/*!
			@enum eRigidBodyScalar
			@brief	Scalar properties for rigid bodies.
		*//*__________________________________________________________________________*/
		enum eRigidBodyScalar
		{
			propAngVelDamp = 0,
			propLinVelDamp,
			propMass
		};
		/*!
			@enum eRigidBodyVector
			@brief	Vector properties for rigid bodies.
		*//*__________________________________________________________________________*/
		enum eRigidBodyVector
		{
			propDimensions = 0,
			propPosition,
			propVeloctity
		};
		/*!
			@enum eRigidBodyQuaternion
			@brief	Quaternion properties for rigid bodies.
		*//*__________________________________________________________________________*/
		enum eRigidBodyQuaternion
		{
			propOrientation = 0
		};

		void		RigidBodyBool(uint32_t id, eRigidBodyBool prop, bool value);
		bool		RigidBodyBool(uint32_t id, eRigidBodyBool prop);
		void		RigidBodyScalar(uint32_t id, eRigidBodyScalar prop, Real value);
		Real		RigidBodyScalar(uint32_t id, eRigidBodyScalar prop);
		void		RigidBodyVector3D(uint32_t id, eRigidBodyVector prop, Vector3D value);
		Vector3D	RigidBodyVector3D(uint32_t id, eRigidBodyVector prop);
		void		RigidBodyQuaternion(uint32_t id, eRigidBodyQuaternion prop, Quaternion value);
		Quaternion	RigidBodyQuaternion(uint32_t id, eRigidBodyQuaternion prop);
		Matrix4x4	RigidBodyTransformationMatrix(uint32_t id);

		uint32_t	AddSpring();
		bool		RemoveSpring(uint32_t id);

		/*!
			@enum eSpringBool
			@brief	Boolean properties for springs.
		*//*__________________________________________________________________________*/
		enum eSpringBool
		{
			propSpringCompressible = 0
		};
		/*!
			@enum eSpringScalar
			@brief	Scalar properties for springs.
		*//*__________________________________________________________________________*/
		enum eSpringScalar
		{
			propSpringStiffness = 0,
			propSpringDamping,
			propSpringRestLength
		};
		/*!
			@enum eSpringVector
			@brief	Vector properties for springs.
		*//*__________________________________________________________________________*/
		enum eSpringVector
		{
			propAttachPoint1 = 0,
			propAttachPoint2
		};
		/*!
			@enum eSpringID
			@brief	Identification properties for springs.
		*//*__________________________________________________________________________*/
		enum eSpringID
		{
			propBody1 = 0,
			propBody2
		};

		void		    SpringBool(uint32_t id, eSpringBool prop, bool value);
		bool		    SpringBool(uint32_t id, eSpringBool prop);
		void		    SpringScalar(uint32_t id, eSpringScalar prop, Real value);
		Real		    SpringScalar(uint32_t id, eSpringScalar prop);
		void		    SpringBody(uint32_t id, eSpringID prop, uint32_t value);
		uint32_t	    SpringBody(uint32_t id, eSpringID prop);
		void		    SpringVector(uint32_t id, eSpringVector prop, Vector3D value);
		Vector3D	    SpringVector(uint32_t id, eSpringVector prop);

		void		    AddImpulse(uint32_t id, Vector3D force);
		void		    StopMoving(uint32_t id);
		void		    AddTwist(uint32_t id, Vector3D twist);
		void		    AddTwist(uint32_t id, Vector3D force, Vector3D position);
		void		    StopSpinning(uint32_t id);

		void		    AddPhysicsCallback(int type, PhysicsCB callbackFn);
		void		    SetGravity(Vector3D value);
		void		    SetMinTimeStep(Real dt);
		void		    Simulate(Real dt);
		virtual void    Update(Real dt, int steps);
		bool		    AtRest(void)const;
        void            AtRest(bool);
		void		    Disturb(void);
		void		    StopAll(void);
		uint32_t	    Kind(uint32_t id)const;
		AuxEngine*		mAuxEngine;

        float           GetMaxLinearVelocity(void)  { return mMaxLinVel; }
        float           GetMaxAngularVelocity(void) { return mMaxAngVel; }
        float           GetDragCoefficient(void)    { return mDragCoeff; }
        float           GetMaxAngularMomentum(void) { return mMaxAngMom; }

	protected:
		
		bool			mIsStatic;
		bool			mWasStatic;
        float           mMaxLinVel;
        float           mMaxAngVel;
        float           mDragCoeff;
        float           mMaxAngMom;
	};

    /*class ShotProject : public Physics::Engine
    {
    public:
        ShotProject() {}
        virtual ~ShotProject() {}
        virtual bool IsAI(void)     { return true; }
        void CopyState(void);
        virtual void Update(Real dt, int steps);
    };*/
}



#define PhysicsEngine (Game::Get()->GetPhysics())
#endif