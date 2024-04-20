/*!
	@file	Ball.h
	@author	Scott Smith
	@date	May 12, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef	__BALL_H__
#define	__BALL_H__

#include "main.h"
#include "Geometry.hpp"
#include "PhysicsDefs.h"
#include "DXSphere.h"
#include "particles.h"

// a behavior function for the ball particle system
struct turbulence_trail
{
	void operator()(D3DXVECTOR3& pos, D3DXVECTOR3& vel)
	{
        pos = pos + vel;
	}
}; 

/*!
 @class		
 @ingroup	ch
 @date		05-12-2004
 @author	Scott
*//*__________________________________________________________________________*/
class Ball : public Physics::Sphere
{
public:
	Ball(Graphics::Renderer *renderer, std::string TexFile, int number, Real radius = 1.f);
	virtual ~Ball();

	/*!
	 @param void 
	 @return 
	*//*__________________________________________________________________________*/
	DXSphere * SphereMesh(void)				{ return &mSphere; }
    /*!
	 @param void 
	 @return 
	*//*__________________________________________________________________________*/
	Particles< DXSphere, turbulence_trail, 32 >* ParticleSystem(void)	{ return mParticles; }
	/*!
	 @param void 
	 @return 
	*//*__________________________________________________________________________*/
	int ID(void)const						{ return mRigidBodyID; }
	/*!
	 @param id 
	 @return 
	*//*__________________________________________________________________________*/
	void ID(const int id)					{ mRigidBodyID = id; }
	/*!
	 @param void 
	 @return 
	*//*__________________________________________________________________________*/
	int Number(void)const						{ return mNumber; }
	/*!
	 @param id 
	 @return 
	*//*__________________________________________________________________________*/
	void Number(const int num)					{ mNumber = num; }
    /*!
	 @param id 
	 @return 
	*//*__________________________________________________________________________*/
	void Pocketed(const bool b)					{ mPocketed = b; }
    /*!
	 @param id 
	 @return 
	*//*__________________________________________________________________________*/
	bool Pocketed(void)					{ return mPocketed; }

protected:
	int                                          mRigidBodyID;
	int                                          mNumber;
    bool                                         mPocketed;
	DXSphere                                     mSphere;
    Particles< DXSphere, turbulence_trail, 32 >* mParticles;

};

#endif