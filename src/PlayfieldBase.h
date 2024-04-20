/*!
	@file	playfield.h
	@author	Scott Smith
	@date	May 18, 2004

	@brief	Interface to playfield.

  (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#ifndef	__PLAYFIELD__
#define	__PLAYFIELD__

#include "main.h"

#include "PhysicsEngine.h"
#include "ball.h"
#include "PlayfieldPocket.h"
#include "PlayfieldCue.h"
#include "GraphicsRenderer.h"

#include "DXLine.h"
#include "DXCylinder.h"

#include "Water.h"
#include "ruleSystem.h"
#include "shotprojection.h"

// size defaults
const int  kPlayfieldDefW = 50;   //!< Default width.
const int  kPlayfieldDefH = 25;   //!< Default height.
const int  kPlayfieldDefD = 100;  //!< Default depth.


/*!
 @class		Playfield
 @ingroup	ch
 @date		05-18-2004
 @author	Scott
*//*__________________________________________________________________________*/
class Playfield
{
public:
	Playfield(float w, float h, float l, Graphics::Renderer *renderer, Physics::Engine *physics);
	~Playfield(void);

	void AddWall(const Geometry::Plane3D& plane);
	void AddPlayfieldToSimulation(Physics::Engine *pe);
	void AddBall(std::string FileName,Geometry::Vector3D pos, int num, float rad = 1.f);
	void AddBoundedPlane(const Geometry::Point3D& p0, const Geometry::Point3D& p1, const Geometry::Point3D& p2);
	void AddPocket(const Geometry::Point3D& p, float sz);
	void RackBalls(eGameType t);
	void LoadBalls(eGameType t);

	Geometry::Vector3D GetBallPosition(Physics::Engine *physics,int ball_number);///@todo make D3DXVECTOR3
	size_t             WallCount(void);
	
	void		BuildArena(void);
	void		Update(Physics::Engine *physics);
    
	void Render( const D3DXVECTOR3& vecPos,const D3DXVECTOR3& vecView );

//private:
	std::vector< Geometry::Plane3D >      mWalls;
	std::vector< Pocket* >                mPockets;
	std::vector< Ball* >                  mBalls;
	std::vector< int >                    mAIBalls;
	std::vector< int >                    mPocketedBalls; 
	std::vector< Physics::BoundedPlane >  mPolys;
	Ball                                 *mGhostBall;

	Graphics::Renderer *mRenderer;
  Cue                *mCue;

  DXSphere           *mShotBall;  //!< Rendered repeatedly along the shot vector.
  ShotProjectInfo    mShotProj;

  float  mWidth;   //! Playfield width (X axis).
  float  mHeight;  //! Playfield height (Y axis).
  float  mDepth;   //! Playfield depth (Z axis).
  
  Water      *mPlanes[6];    //! Water planes.
  DXCylinder *mRails[12];    //! Rails.
  DXSphere   *mRailCaps[8];  //! Rail caps.
  DXLine     *mShotLineR;    //! Shot line renderer.
  DXLine     *mBlueLineR;    //! Blue guide line renderer.

	void PlayfieldUpdate(void);
};

int GetBallNumber(int rigidbody_ID);
Ball * GetBallByNumber(int num);

#endif