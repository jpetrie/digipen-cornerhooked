/*!
	@file	ShotProjection.cpp
	@author	Scott Smith
	@date	November 6, 2004

	@brief	Projects a ball onto the closest object in the line of sight.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"
#include "ShotProjection.h"
#include "game.h"
#include "physics.h"
#include "PhysicsAux.h"

ShotProjectInfo ShotProject::Project(void)
{
    ShotProjectInfo ret;
    float t = kMax;
    ret.time = kMax;
    D3DXVECTOR3 cam_pos, view;
    Game::Get()->GetCamera()->GetTargetVector(&cam_pos);
    view = Game::Get()->GetMyShotVector();
    Geometry::Point3D camPoint(cam_pos.x, cam_pos.y, cam_pos.z);
    Geometry::Vector3D camVec(camPoint[0], camPoint[1], camPoint[2]);
    Geometry::Ray3D ray(camPoint, Geometry::Vector3D(view.x, view.y, view.z));
    // go through all of the bodies in the physics engine
    Physics::RigidBodyMap::iterator it = Game::Get()->GetPhysics()->mAuxEngine->mBodies.begin();

    for(;it != Game::Get()->GetPhysics()->mAuxEngine->mBodies.end(); ++it)
    {
        // handle a ray intersection with a sphere
        if(it->second->mCollideGeom->Kind() == Physics::kC_Sphere)
        {
            // dont't consider the cueball
            if(GetBallNumber(it->first) == 0)
                continue;
            Geometry::Vector3D c = Game::Get()->GetPhysics()->RigidBodyVector3D(it->first, Physics::Engine::eRigidBodyVector::propPosition);
            Geometry::Sphere3D s(Geometry::Point3D(c[0], c[1], c[2]), 2.f);
            if(Geometry::Intersects(ray, s, 0, &t))
            {
                if(t > 0.f && t < ret.time)
                {
                    ret.time = t;
                    ret.ID = it->first;
                    ret.body = it->second;
                    Geometry::Vector3D u = (camVec + t*ray.direction.normal());
                    ret.project = D3DXVECTOR3(u[0], u[1], u[2]);
                }
            }
        }
        else if(it->second->mCollideGeom->Kind() == Physics::kC_Plane)
        {
            if(Geometry::Intersects(ray, static_cast<Physics::Plane*>(it->second->mCollideGeom)->mPlane, 0, &t))
            {
                if(t > 0.f && t < ret.time)
                {
                    ret.time = t;
                    ret.ID = it->first;
                    ret.body = it->second;
                    Geometry::Vector3D u = (camVec + t*ray.direction.normal()) + (static_cast<Physics::Plane*>(it->second->mCollideGeom)->mPlane.normal().normal());
                    ret.project = D3DXVECTOR3(u[0], u[1], u[2]);
                }
            }
        }
        /*else
        {
            ret.time = kMax;
            ret.ID = -1;
            ret.body = 0;
            ret.project = D3DXVECTOR3(0,0,0);
        }*/
    }
    return ret;
}