/*!
	@file	Ball.cpp
	@author	Scott Smith
	@date	May 12, 2004

	@brief	Internal definitions.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/


#include "main.h"

#include "ball.h"

/*!
 @param radius The radius of the ball.
*//*__________________________________________________________________________*/
Ball::Ball(Graphics::Renderer *renderer, std::string TexFile, int num, Real radius)
: Sphere(radius),mRigidBodyID(0),mPocketed(false),mNumber(num),mSphere(renderer,TexFile),
mParticles(new Particles< DXSphere, turbulence_trail, 32 >(renderer))
 {}
/*!
	@brief	Cleanup the mesh.
*//*__________________________________________________________________________*/
Ball::~Ball()
{

}

