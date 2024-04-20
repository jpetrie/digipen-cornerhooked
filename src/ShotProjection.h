/*!
	@file	ShotProjection.h
	@author	Scott Smith
	@date	November 6, 2004

	@brief	Projects a ball onto the closest object in the line of sight.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"
#include "geometry.hpp"
#include "rigidbody.h"

#ifndef __SHOT_PROJECT_H__
#define __SHOT_PROJECT_H__

struct ShotProjectInfo
{
    uint32_t            ID;
    Physics::RigidBody* body;
    D3DXVECTOR3         project;
    double              time;
};

class ShotProject
{
public:
    // ct & dt
    ShotProject() {}
    ~ShotProject() {}

    ShotProjectInfo Project(void);
};

#endif