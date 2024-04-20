/*!
	@file	playfield.cpp
	@author	Scott Smith
	@date	May 18, 2004

	@brief	Implementation of playfield.

  (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/

#include "main.h"

#include "DXCircle.h"
#include "PlayfieldBase.h"
#include "game.h"
#include "Profiler.h"
#include "nsl_random.h"
#include "AIPlayer.h"
#include "physics.h"
#include "physicsAux.h"
#include "UIPowerMeter.h"
#include "MathDefs.h"
#include "DXCylinder.h"
#include "SoundEngine.h"

namespace
{  
  const int  kFrontIdx  = 0;
  const int  kBackIdx   = 1;
  const int  kTopIdx    = 2;
  const int  kBottomIdx = 3;
  const int  kLeftIdx   = 5;
  const int  kRightIdx  = 4;
  
  const int  kRailTF =  0;  // Top front.
  const int  kRailTB =  1;  // Top back.
  const int  kRailTL =  2;  // Top left.
  const int  kRailTR =  3;  // Top right.
  const int  kRailBF =  4;  // Bottom front.
  const int  kRailBB =  5;  // Bottom back.
  const int  kRailBL =  6;  // Bottom left.
  const int  kRailBR =  7;  // Bottom right.
  const int  kRailFL =  8;  // Front left.
  const int  kRailFR =  9;  // Front right.
  const int  kRailAL = 10;  // Aft left.
  const int  kRailAR = 11;  // Aft right.
  
  const float kPlaneRot_90  = 1.57f;             // 90 degrees.
  const float kPlaneRot_180 = 2 * kPlaneRot_90;  // 180 degrees.
  const float kPlaneRot_270 = 3 * kPlaneRot_90;  // 270 degrees.
  
  const int  kXDensity = 1;
  const int  kYDensity = 1;
  
  const float kPocketSz = 6.0f;
  const float kRailSz   = 0.4f;
}


const float ds = tan(Math::DegToRad(30.f));
const float dl = tan(Math::DegToRad(60.f)) - tan(Math::DegToRad(30.f));
const float r = 1.2f;

int GetBallNumber(int rigidbody_ID)
{
    std::vector<Ball*> game_balls = Game::Get()->GetPlayfield()->mBalls;
    for(unsigned int i = 0; i < game_balls.size(); ++i)
    {
        if(game_balls[i]->ID() == rigidbody_ID)
            return game_balls[i]->Number();
    }
    return -1;
    /*if(Game::Get()->GetPhysics()->mAuxEngine->mBodies.count(rigidbody_ID))
    {
        for(unsigned int i = 0; i < Game::Get()->GetPlayfield()->mBalls.size(); ++i)
        {
            if(Game::Get()->GetPhysics()->mAuxEngine->mBodies[Game::Get()->GetPlayfield()->mBalls[i]->ID()] == 
                Game::Get()->GetPhysics()->mAuxEngine->mBodies[rigidbody_ID])
            {
                Game::Get()->GetPlayfield()->mBalls[i]->Number();
            }
        }
        return -1;
    }
    else
    {
        return -1;
    }*/
}
Ball * GetBallByNumber(int num)
{
    std::vector< Ball * > game_balls = Game::Get()->GetPlayfield()->mBalls;
    for(unsigned int i = 0; i < game_balls.size(); ++i)
    {
        if(game_balls[i]->Number() == num)
            return game_balls[i];
    }
    return 0;
}

std::vector< Geometry::Vector3D > balls;
// random number generator
random	rng(34357);

int BaseCount(int deep)
{
	if(deep == 1)
		return 1;
	return deep + BaseCount(deep - 1);
}
int BallCount(int deep)
{
	int ret = 0;
	for(int i = 1; i <= deep; ++i)
	{
		ret += BaseCount(i);
	}
	return ret;
}

static void BuildRow(int n, Geometry::Vector3D p)
{
	for(int i = 0; i < n; ++i)
	{
		balls.push_back(p);
		p[0] += 2 * r + .1f;
	}
}

static Geometry::Vector3D BuildLayer(int n, Geometry::Vector3D last)
{
	Geometry::Vector3D ret = last;
	for(int i = n; i > 0; --i)
	{
		BuildRow(i, last);
		last[0] += r;
		last[1] -= dl + ds;/*2 * dl*/ //+ rng.rand_float(-.2f, .2f);
	}
	ret[0] -= r;
	ret[1] += ds;
	ret[2] += sqrt(3.f) * r - .1f;
	return ret;
}


Playfield::Playfield(float w,float h,float l,Graphics::Renderer *renderer,Physics::Engine *pPEngine)
: mRenderer(renderer),
  mWidth(w),mHeight(h),mDepth(l),
  mShotBall(0)
{
int  water_res = ::GetPrivateProfileInt("Water","Resolution",1,"data/config/user.ini");

	// Construct the water planes.
  mPlanes[kTopIdx]    = new Water(renderer,mWidth,mDepth,water_res,water_res);
  mPlanes[kBottomIdx] = new Water(renderer,mWidth,mDepth,water_res,water_res);
  mPlanes[kLeftIdx]   = new Water(renderer,mDepth,mHeight,water_res,water_res);
  mPlanes[kRightIdx]  = new Water(renderer,mDepth,mHeight,water_res,water_res);
  mPlanes[kFrontIdx]  = new Water(renderer,mWidth,mHeight,water_res,water_res);
  mPlanes[kBackIdx]   = new Water(renderer,mWidth,mHeight,water_res,water_res);
	
	// Orient and position the planes. The normals should face inward.
	// Since orientation is expressed as a quaternion, but it is more
	// convient to orient via axis-angle rotation, we must convert
	// the parameters to SetRotation(). 
	mPlanes[kTopIdx]->SetWorldNormal(D3DXVECTOR3(0.0,-1.0,0.0));
	mPlanes[kTopIdx]->SetTranslation(0.0f,mHeight / 2.0f,0.0f);
	mPlanes[kTopIdx]->SetRotation(sin(-kPlaneRot_90 / 2.0f) * 1.0f,
	                              sin(-kPlaneRot_90 / 2.0f) * 0.0f,
	                              sin(-kPlaneRot_90 / 2.0f) * 0.0f,
	                              cos(-kPlaneRot_90 / 2.0f));
	mPlanes[kBottomIdx]->SetWorldNormal(D3DXVECTOR3(0.0,1.0,0.0));
	mPlanes[kBottomIdx]->SetTranslation(0.0f,-mHeight / 2.0f,0.0f);
	mPlanes[kBottomIdx]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                                 sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                                 sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                                 cos(kPlaneRot_90 / 2.0f));
	mPlanes[kLeftIdx]->SetWorldNormal(D3DXVECTOR3(1.0,0.0,0.0));
	mPlanes[kLeftIdx]->SetTranslation(-mWidth / 2.0f,0.0f,0.0f);
	mPlanes[kLeftIdx]->SetRotation(sin(kPlaneRot_270 / 2.0f) * 0.0f,
	                               sin(kPlaneRot_270 / 2.0f) * 1.0f,
	                               sin(kPlaneRot_270 / 2.0f) * 0.0f,
	                               cos(kPlaneRot_270 / 2.0f));
  mPlanes[kRightIdx]->SetWorldNormal(D3DXVECTOR3(-1.0,0.0,0.0));
	mPlanes[kRightIdx]->SetTranslation(mWidth / 2.0f,0.0f,0.0f);
	mPlanes[kRightIdx]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                                sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                                sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                                cos(kPlaneRot_90 / 2.0f));
	mPlanes[kFrontIdx]->SetWorldNormal(D3DXVECTOR3(0.0,0.0,1.0));                      
	mPlanes[kFrontIdx]->SetTranslation(0.0f,0.0f,-mDepth / 2.0f);
	mPlanes[kFrontIdx]->SetRotation(sin(kPlaneRot_180 / 2.0f) * 0.0f,
	                                sin(kPlaneRot_180 / 2.0f) * 1.0f,
	                                sin(kPlaneRot_180 / 2.0f) * 0.0f,
	                                cos(kPlaneRot_180 / 2.0f));
	                                
	// The back plane needs no orientation.
	mPlanes[kBackIdx]->SetWorldNormal(D3DXVECTOR3(0.0,0.0,-1.0)); 
	mPlanes[kBackIdx]->SetTranslation(0.0f,0.0f,mDepth / 2.0f);
	
	// Create the rails.
	mRails[kRailTF] = new DXCylinder(mRenderer,kRailSz,mWidth);
	mRails[kRailTB] = new DXCylinder(mRenderer,kRailSz,mWidth);
	mRails[kRailTL] = new DXCylinder(mRenderer,kRailSz,mDepth);
	mRails[kRailTR] = new DXCylinder(mRenderer,kRailSz,mDepth);
  mRails[kRailBF] = new DXCylinder(mRenderer,kRailSz,mWidth);
	mRails[kRailBB] = new DXCylinder(mRenderer,kRailSz,mWidth);
	mRails[kRailBL] = new DXCylinder(mRenderer,kRailSz,mDepth);
	mRails[kRailBR] = new DXCylinder(mRenderer,kRailSz,mDepth);
	mRails[kRailFL] = new DXCylinder(mRenderer,kRailSz,mHeight);
	mRails[kRailFR] = new DXCylinder(mRenderer,kRailSz,mHeight);
	mRails[kRailAL] = new DXCylinder(mRenderer,kRailSz,mHeight);
	mRails[kRailAR] = new DXCylinder(mRenderer,kRailSz,mHeight);
	
	// Apply translations and orientations to position the rails.
	mRails[kRailTL]->SetTranslation(-mWidth / 2.0f,mHeight / 2.0f,0.0f);
	mRails[kRailTR]->SetTranslation(mWidth / 2.0f,mHeight / 2.0f,0.0f);
	mRails[kRailBL]->SetTranslation(-mWidth / 2.0f,-mHeight / 2.0f,0.0f);
	mRails[kRailBR]->SetTranslation(mWidth / 2.0f,-mHeight / 2.0f,0.0f);
  mRails[kRailTF]->SetTranslation(0.0f,mHeight / 2.0f,-mDepth / 2.0f);
  mRails[kRailTF]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailTB]->SetTranslation(0.0f,mHeight / 2.0f,mDepth / 2.0f);
  mRails[kRailTB]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailBF]->SetTranslation(0.0f,-mHeight / 2.0f,-mDepth / 2.0f);
  mRails[kRailBF]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailBB]->SetTranslation(0.0f,-mHeight / 2.0f,mDepth / 2.0f);
  mRails[kRailBB]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailFL]->SetTranslation(-mWidth / 2.0f,0.0f,-mDepth / 2.0f);
  mRails[kRailFL]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailAL]->SetTranslation(-mWidth / 2.0f,0.0f,mDepth / 2.0f);
  mRails[kRailAL]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailFR]->SetTranslation(mWidth / 2.0f,0.0f,-mDepth / 2.0f);
  mRails[kRailFR]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	mRails[kRailAR]->SetTranslation(mWidth / 2.0f,0.0f,mDepth / 2.0f);
  mRails[kRailAR]->SetRotation(sin(kPlaneRot_90 / 2.0f) * 1.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             sin(kPlaneRot_90 / 2.0f) * 0.0f,
	                             cos(kPlaneRot_90 / 2.0f));
	
	// Create the rail caps.
	mRailCaps[0] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[1] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[2] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[3] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[4] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[5] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[6] = new DXSphere(mRenderer,"data/ghost.png");
	mRailCaps[7] = new DXSphere(mRenderer,"data/ghost.png");

	mRailCaps[0]->SetTranslation(-mWidth / 2.0f,mHeight / 2.0f,mDepth / 2.0f);
	mRailCaps[1]->SetTranslation(mWidth / 2.0f,mHeight / 2.0f,mDepth / 2.0f);
	mRailCaps[2]->SetTranslation(-mWidth / 2.0f,-mHeight / 2.0f,mDepth / 2.0f);
	mRailCaps[3]->SetTranslation(mWidth / 2.0f,-mHeight / 2.0f,mDepth / 2.0f);
	mRailCaps[4]->SetTranslation(-mWidth / 2.0f,mHeight / 2.0f,-mDepth / 2.0f);
	mRailCaps[5]->SetTranslation(mWidth / 2.0f,mHeight / 2.0f,-mDepth / 2.0f);
	mRailCaps[6]->SetTranslation(-mWidth / 2.0f,-mHeight / 2.0f,-mDepth / 2.0f);
	mRailCaps[7]->SetTranslation(mWidth / 2.0f,-mHeight / 2.0f,-mDepth / 2.0f);

	
	for(unsigned int i = 0; i < 8; ++i)
	{
	  mRailCaps[i]->SetScale(0.5f,0.5f,0.5f);
	  mRailCaps[i]->UseShader("BallRailV11P11");
	}
	
	// Create the cue and related objects.
	mCue      = new Cue(mRenderer);
	mShotBall = new DXSphere(mRenderer,"data/ghost.png");
	mShotBall->UseShader("BallGhostV11P11");
	
	mShotLineR = new DXLine(mRenderer,0xFFFF0000,0x00FF0000);
	mBlueLineR = new DXLine(mRenderer,0x7F0000FF,0x000000FF);
	
	mShotProj.ID = 0;
	mShotProj.body = 0;
	
	BuildArena();

	///@todo If we have multiple physics engines, won't we need to add balls to all somehow?
	//       Does this make the physics engine parameter pointless?	
	AddPlayfieldToSimulation(pPEngine);
}

Playfield::~Playfield(void) 
{
	for(unsigned int i = 0; i < mBalls.size(); ++i)
		delete mBalls[i];
	mBalls.clear();
	
	for(unsigned int i = 0; i < 6; ++i)
	  SAFE_DELETE(mPlanes[i]);
	for(unsigned int i = 0; i < 12; ++i)
	  SAFE_DELETE(mRails[i]);
	
	SAFE_DELETE(mBlueLineR);
	SAFE_DELETE(mShotLineR);
	SAFE_DELETE(mCue);
  SAFE_DELETE(mShotBall);
}


/********************************************************/
/*!
	Renders BoxPlayfield.
*/
/********************************************************/
void Playfield::Render(const D3DXVECTOR3 &vecPos,const D3DXVECTOR3 &vecView)
{
Geometry::Vector3D  org        = GetBallPosition(Game::Get()->GetPhysics(),0);
D3DXVECTOR3         shotVector = Game::Get()->GetMyShotVector();
D3DXVECTOR3         origin(org[0],org[1],org[2]);

    mShotProj = Game::Get()->GetAimBall()->Project();
  

  // Batch-render the balls.
  DXSphere::EnterRenderState();
  if(Game::Get()->DoesSessionExist() && 
     Game::Get()->GetSession()->IsPlaying() &&
     Game::Get()->IsMyTurn() && Game::Get()->GetPhysics()->AtRest() && 
     Game::Get()->DrawLitBalls())
  {
  std::vector< int >  legalBalls = Game::Get()->GetSession()->GetRules()->GetLegalBalls(Game::Get()->GetSession()->CurrentTurn());
    
    for(unsigned int i = 0; i < mBalls.size(); ++i)
    {
      if(i == 0)
      {
        mBalls[i]->SphereMesh()->UseShader("BallLiteV11P11");
      }
      else if(std::find(legalBalls.begin(),legalBalls.end(),i) != legalBalls.end())
        mBalls[i]->SphereMesh()->UseShader("BallLiteV11P11");
      else
        mBalls[i]->SphereMesh()->UseShader("BallDarkV11P11");
      
      if(!mBalls[i]->Pocketed())
        mBalls[i]->SphereMesh()->Render(vecPos,vecView);
    
 //     if(mBalls[i]->Pocketed() && i == 0)
 //       Game::Get()->WriteMessage("CUE IS SUNK.");
    }
  }
  else
  {
    for(unsigned int i = 0; i < mBalls.size(); ++i)
    {
      mBalls[i]->SphereMesh()->UseShader("BallLiteV11P11");

      if(!mBalls[i]->Pocketed())
        mBalls[i]->SphereMesh()->Render(vecPos,vecView);
        
//      if(GetBallByNumber(0)->Pocketed())
//        Game::Get()->WriteMessage("STUPID");
    }  
  }
  DXSphere::LeaveRenderState();
  
  // Render the planes.
  Water::EnterRenderState();
  for(unsigned int i = 0 ; i < 6; ++i)
    mPlanes[i]->Render(vecPos,vecView);
  Water::LeaveRenderState();
  
  // Render the rails.
bool  railBools[12] = { 0 };

  DXCylinder::EnterRenderState();
  if(mPlanes[kFrontIdx]->isRendered)
  {
    railBools[kRailFR] = true;
    railBools[kRailFL] = true;
    railBools[kRailTF] = true;
    railBools[kRailBF] = true;
  }
  if(mPlanes[kBackIdx]->isRendered)
  {
    railBools[kRailAR] = true;
    railBools[kRailAL] = true;
    railBools[kRailTB] = true;
    railBools[kRailBB] = true;
  }
  if(mPlanes[kTopIdx]->isRendered)
  {
    railBools[kRailTR] = true;
    railBools[kRailTL] = true;
    railBools[kRailTB] = true;
    railBools[kRailTF] = true;
  }
  if(mPlanes[kBottomIdx]->isRendered)
  {
    railBools[kRailBR] = true;
    railBools[kRailBL] = true;
    railBools[kRailBB] = true;
    railBools[kRailBF] = true;
  }
  if(mPlanes[kLeftIdx]->isRendered)
  {
    railBools[kRailTL] = true;
    railBools[kRailBL] = true;
    railBools[kRailFL] = true;
    railBools[kRailAL] = true;
  }
  if(mPlanes[kRightIdx]->isRendered)
  {
    railBools[kRailTR] = true;
    railBools[kRailBR] = true;
    railBools[kRailFR] = true;
    railBools[kRailAR] = true;
  }
  for(unsigned int i = 0; i < 12; ++i)
    if(railBools[i])
      mRails[i]->Render(vecPos,vecView);
  
  DXCylinder::LeaveRenderState();
  DXSphere::EnterRenderState();
  for(unsigned int i = 0; i < 8; ++i)
	  mRailCaps[i]->Render(vecPos,vecView);
  DXSphere::LeaveRenderState();
  
  // Render the pockets.
  for(unsigned int i = 0; i < mPockets.size(); ++i)
    mPockets[i]->Render(vecPos,vecView);
  
  // Render various aiming aids if the current game turn is that of
  // the local machine.
  
  // For ball-in-hand.
  if(Game::Get()->DoesSessionExist() && 
     Game::Get()->GetSession()->IsPlaying() &&
     Game::Get()->IsMyTurn() && Game::Get()->GetPhysics()->AtRest() &&
     Game::Get()->GetSession()->CurrentState() == Game::Get()->GetSession()->BallInHandSID())
  {
    // Left/right plane line.
    mBlueLineR->DrawLine(mWidth / 2,org[1],org[2],org[0],org[1],org[2]);
    mBlueLineR->DrawLine(-mWidth / 2,org[1],org[2],org[0],org[1],org[2]);
    
    // Top/bottom plane line.
    mBlueLineR->DrawLine(org[0],mHeight / 2,org[2],org[0],org[1],org[2]);
    mBlueLineR->DrawLine(org[0],-mHeight / 2,org[2],org[0],org[1],org[2]);
    
    // Front plane line (none for the opposite plane).
    mBlueLineR->DrawLine(org[0],org[1],-mDepth / 2,org[0],org[1],org[2]);
  }
  
  // For shot lineup.
  if(Game::Get()->DoesSessionExist() && 
     Game::Get()->GetSession()->IsPlaying() &&
     Game::Get()->IsMyTurn() && Game::Get()->GetPhysics()->AtRest() &&
     Game::Get()->GetSession()->CurrentState() == Game::Get()->GetSession()->ShotLineupSID())
  {
    // Render the cue.
  UIPowerMeter *powerMeter = static_cast< UIPowerMeter* >(Game::Get()->GetScreen()->GetElement(kUI_GPPowerMeterName));
  
    if(powerMeter != 0)
    {
    float  cueDist = 0.5f + (3.0f * powerMeter->GetPower());
    
      if(Game::Get()->GetSession()->CurrentState() == Game::Get()->GetSession()->WatchShotSID())
        mCue->Render(shotVector,Game::Get()->GetShotAnimData().cueTip,0.0f);
      else
        mCue->Render(shotVector,origin,cueDist);
    }
    
  Geometry::Vector3D  org(origin.x, origin.y, origin.z);
  Geometry::Vector3D  shotVec(shotVector.x,shotVector.y,shotVector.z);

    // Render projected shot.
    mShotBall->SetTranslation(mShotProj.project[0], mShotProj.project[1], mShotProj.project[2]);
    mShotBall->Render(vecPos,vecView);
    
    // Render shot line.
    if(mShotProj.body)
    {
      mShotLineR->DrawLine(origin.x,origin.y,origin.z,
                          mShotProj.project[0], mShotProj.project[1], mShotProj.project[2]);
      
      // Render reflect line.
    Geometry::Vector3D  contactVec; 
    Geometry::Vector3D  contactPos;

      if(mShotProj.body->mCollideGeom != 0 && mShotProj.body->mCollideGeom->Kind() == Physics::kC_Plane)
      {
      Geometry::Vector3D  nrmlVec = static_cast< Physics::Plane* >(mShotProj.body->mCollideGeom)->mPlane.normal();
      
        contactVec = Geometry::Vector3D(mShotProj.project.x,mShotProj.project.y,mShotProj.project.z);
        
        // Compute reflection vector, given the entering ray direction i, and the surface normal n,
        // to be v = i - 2 * dot(i, n) * n.
      Geometry::Vector3D  reflVec = shotVec - (2 * Geometry::Dot(shotVec,nrmlVec)) * nrmlVec;
        
        reflVec = reflVec.normal() * 10.0f;
        mShotLineR->DrawLine(contactVec[0],contactVec[1],contactVec[2],
                             contactVec[0] + reflVec[0],contactVec[1] + reflVec[1],contactVec[2] + reflVec[2]);
        Game::Get()->lookingAtBall = 0;
      }
      else
      {
      DXSphere *ball = GetBallByNumber(GetBallNumber(mShotProj.ID))->SphereMesh();
        
        ball->UseShader("BallV11P11");
        ball->Render(vecPos,vecView);
        
        // Save the ball number we're looking at.
        Game::Get()->lookingAtBall = GetBallNumber(mShotProj.ID);
        
        contactVec = Game::Get()->GetPhysics()->RigidBodyVector3D(mShotProj.ID,Physics::Engine::eRigidBodyVector::propPosition);

        contactPos = contactVec;
        contactVec = contactPos - Geometry::Vector3D(mShotProj.project.x,mShotProj.project.y,mShotProj.project.z);
        contactVec = contactVec.normal();
        contactVec *= 10.0f;
        
        mShotLineR->DrawLine(contactPos[0],contactPos[1],contactPos[2],
                             contactPos[0] + contactVec[0],contactPos[1] + contactVec[1],contactPos[2] + contactVec[2]);
      }
      
      // Ghost ball.
      if(Game::Get()->GetPhysics()->AtRest() && Game::Get()->GetSession()->CurrentTurn() == Game::Get()->GetMyTurn())
      {
        if(Game::Get()->DrawGhost())
        {
        Shot shotGhost = (Game::Get()->GetSession()->GetPlayer(Game::Get()->GetSession()->CurrentTurn()))->GetBestShot();
        	
	        Game::Get()->GetPlayfield()->mGhostBall->SphereMesh()->SetTranslation(shotGhost.p[0],shotGhost.p[1],shotGhost.p[2]);
          mGhostBall->SphereMesh()->Render(vecPos,vecView);
        }
      }
    }
  }
    
  // For shot calling.
  if(Game::Get()->DoesSessionExist() && 
     Game::Get()->GetSession()->IsPlaying() &&
     Game::Get()->IsMyTurn() && Game::Get()->GetPhysics()->AtRest() &&
     Game::Get()->GetSession()->CurrentState() == Game::Get()->GetSession()->CallShotSID())
  {
  D3DXVECTOR3  viewVecDX;
  D3DXVECTOR3  viewLocDX;
  
    // Test and see if the view vector strikes a pocket plane.
    Game::Get()->GetCamera()->GetViewVector(&viewVecDX);
    Game::Get()->GetCamera()->GetPosVector(&viewLocDX);
  
  Geometry::Point3D   viewLoc(viewLocDX.x,viewLocDX.y,viewLocDX.z);
  Geometry::Vector3D  viewVec(Geometry::Point3D(org[0],org[1],org[2]) - viewLoc);//(viewVecDX.x,viewVecDX.y,viewVecDX.z);
  
  Geometry::Ray3D     viewRay(Geometry::Point3D(org[0],org[1],org[2]),viewVec);
    
    
    for(unsigned int i = 0; i < 1; ++i)
    {
      Geometry::Point3D  strikePt;
      //float              strikeTm;

      Geometry::Point3D xPoint(mPockets[i]->mCornerX.x, mPockets[i]->mCornerX.y, mPockets[i]->mCornerX.z);
      Geometry::Point3D yPoint(mPockets[i]->mCornerY.x, mPockets[i]->mCornerY.y, mPockets[i]->mCornerY.z);
      Geometry::Point3D zPoint(mPockets[i]->mCornerZ.x, mPockets[i]->mCornerZ.y, mPockets[i]->mCornerZ.z);
      
      Geometry::Box3D  box(xPoint,yPoint);
      
      Geometry::Triangle3D plane(xPoint, yPoint, zPoint);
      ///mPockets[i]->PhysicsPlanes()[0].mPolygon;//
      if(Geometry::Intersects(viewRay, plane, &strikePt) && plane.contains(strikePt))
      {
        mGhostBall->SphereMesh()->SetTranslation(strikePt[0],strikePt[1],strikePt[2]);
        mGhostBall->SphereMesh()->Render(vecPos,vecView);
        mPockets[i]->isLit = true;
      }
      else
        mPockets[i]->isLit = false;
    }
  }
}

/********************************************************/
/*!
	Adds walls used by BoxPlayfield.
*/
/********************************************************/
void Playfield::BuildArena()
{
float fHalfHeight = mHeight / 2.0f;
float fHalfWidth  = mWidth / 2.0f;
float fHalfDepth  = mDepth / 2.0f;

	AddWall(Geometry::Plane3D(0.0, -1, 0, fHalfHeight));
	AddWall(Geometry::Plane3D(0, 1, 0, fHalfHeight));
	AddWall(Geometry::Plane3D(-1, 0, 0, fHalfWidth));
	AddWall(Geometry::Plane3D(1, 0, 0, fHalfWidth));
	AddWall(Geometry::Plane3D(0.0, 0, -1, fHalfDepth));
	AddWall(Geometry::Plane3D(0.0, 0, 1, fHalfDepth));
	
	// btr
	AddPocket(Geometry::Point3D(fHalfWidth, fHalfHeight, fHalfDepth), kPocketSz);
	// btr
	AddPocket(Geometry::Point3D(-fHalfWidth, fHalfHeight, fHalfDepth), kPocketSz);
	// bbr
	AddPocket(Geometry::Point3D(fHalfWidth, -fHalfHeight, fHalfDepth), kPocketSz);
	// bbl
	AddPocket(Geometry::Point3D(-fHalfWidth, -fHalfHeight, fHalfDepth), kPocketSz);

	// ftr
	AddPocket(Geometry::Point3D(-fHalfWidth, fHalfHeight, -fHalfDepth), kPocketSz);
	// ftr
	AddPocket(Geometry::Point3D(fHalfWidth, fHalfHeight, -fHalfDepth), kPocketSz);
	// fbr
	AddPocket(Geometry::Point3D(-fHalfWidth, -fHalfHeight, -fHalfDepth), kPocketSz);
	// fbl
	AddPocket(Geometry::Point3D(fHalfWidth, -fHalfHeight, -fHalfDepth), kPocketSz);
}

void Playfield::Update(Physics::Engine* pPEngine)
{
ProfileFn;
	Geometry::Vector3D pos;
	Quaternion q;
	
	// Update the water.
  for(unsigned int i = 0 ; i < 6; ++i)
    mPlanes[i]->Update();
	
	// Update the cue.
	mCue->Update();
	
	for(unsigned int i = 0; i < mBalls.size(); ++i)
	{
		pos = pPEngine->RigidBodyVector3D(mBalls[i]->ID(), Physics::Engine::eRigidBodyVector::propPosition);
		q = pPEngine->RigidBodyQuaternion(mBalls[i]->ID(), Physics::Engine::eRigidBodyQuaternion::propOrientation);
		mBalls[i]->SphereMesh()->SetTranslation(pos[0],pos[1],pos[2]);
		mBalls[i]->SphereMesh()->SetRotation(q.X(),q.Y(),q.Z(),q.W());
	}
}
/*!
 @param plane A plane to add to the simulation as a wall.
*//*__________________________________________________________________________*/
void Playfield::AddWall(const Geometry::Plane3D& plane)
{
	mWalls.push_back(plane);
}
/*!
 @param p0 
 @param p1 
 @param p2 
*//*__________________________________________________________________________*/
void Playfield::AddBoundedPlane(const Geometry::Point3D& p0, const Geometry::Point3D& p1, const Geometry::Point3D& p2)
{
	mPolys.push_back(Physics::BoundedPlane(p0, p1, p2));
}

/*!
 @param pos 
 @param rad 
 @param mass 
*//*__________________________________________________________________________*/
void Playfield::AddBall(std::string TexFile, Geometry::Vector3D /*pos*/, int num, float /*rad*/)
{
	int ball_num = mBalls.size();
	mBalls.push_back(new Ball(mRenderer, TexFile, num));
	mBalls[ball_num]->ID(Game::Get()->GetPhysics()->AddRigidBodySphere(1.0));
	Game::Get()->GetPhysics()->RigidBodyBool(mBalls[ball_num]->ID(), Physics::Engine::eRigidBodyBool::propCollidable, true);
	Game::Get()->GetPhysics()->RigidBodyBool(mBalls[ball_num]->ID(), Physics::Engine::eRigidBodyBool::propActive, true);
	Game::Get()->GetPhysics()->RigidBodyBool(mBalls[ball_num]->ID(), Physics::Engine::eRigidBodyBool::propTranslatable, true);
	Game::Get()->GetPhysics()->RigidBodyBool(mBalls[ball_num]->ID(), Physics::Engine::eRigidBodyBool::propUseGravity, true);
	Game::Get()->GetPhysics()->RigidBodyScalar(mBalls[ball_num]->ID(), Physics::Engine::eRigidBodyScalar::propMass, .5);
	mAIBalls.push_back(0);
	mAIBalls[ball_num] = Game::Get()->GetAIPhysics()->AddRigidBodySphere(1.0);
	Game::Get()->GetAIPhysics()->RigidBodyBool(mAIBalls[ball_num], Physics::Engine::eRigidBodyBool::propCollidable, true);
	Game::Get()->GetAIPhysics()->RigidBodyBool(mAIBalls[ball_num], Physics::Engine::eRigidBodyBool::propActive, true);
	Game::Get()->GetAIPhysics()->RigidBodyBool(mAIBalls[ball_num], Physics::Engine::eRigidBodyBool::propTranslatable, true);
	Game::Get()->GetAIPhysics()->RigidBodyBool(mAIBalls[ball_num], Physics::Engine::eRigidBodyBool::propUseGravity, true);
	Game::Get()->GetAIPhysics()->RigidBodyScalar(mAIBalls[ball_num], Physics::Engine::eRigidBodyScalar::propMass, .5);
	
	
	
}
/*!
	@param p 
	@param sz 
*//*__________________________________________________________________________*/
void Playfield::AddPocket(const Geometry::Point3D& p, float sz)
{
Pocket *pocket = new Pocket(Game::Get()->GetWindow()->GetRenderer(),D3DXVECTOR3(p[0],p[1],p[2]),sz);

	mPockets.push_back(pocket);
}
/*!
 @param pe A physics engine to add the walls to.
 @note	Don't call this more than once..  
*//*__________________________________________________________________________*/
void Playfield::AddPlayfieldToSimulation(Physics::Engine *pe)
{
	//{
		std::vector< Geometry::Plane3D >::iterator it = mWalls.begin();
		while(it != mWalls.end())
		{
			int key = pe->AddRigidBodyPlane(*it);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propCollidable, true);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propActive, true);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propTranslatable, false);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propUseGravity, false);
			++it;
		}
		/*std::vector< Physics::BoundedPlane >::iterator it3 = mPolys.begin();
		while(it3 != mPolys.end())
		{
			int key = pe->AddRigidBodyBoundedPlane(*it3);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propCollidable, true);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propActive, true);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propTranslatable, false);
			pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propUseGravity, false);
			++it3;
		}*/
		/*std::vector< Ball* >::iterator it2 = mBalls.begin();
		while(it2 != mBalls.end())
		{
			(*it2)->ID(pe->AddRigidBodySphere((*it2)->mRadius));
			pe->RigidBodyBool((*it2)->ID(), Physics::Engine::eRigidBodyBool::propCollidable, true);
			pe->RigidBodyBool((*it2)->ID(), Physics::Engine::eRigidBodyBool::propActive, true);
			pe->RigidBodyBool((*it2)->ID(), Physics::Engine::eRigidBodyBool::propTranslatable, true);
			pe->RigidBodyBool((*it2)->ID(), Physics::Engine::eRigidBodyBool::propUseGravity, true);
			pe->RigidBodyScalar((*it2)->ID(), Physics::Engine::eRigidBodyScalar::propMass, 1.0);
			++it2;
		}*/
		std::vector< Pocket* >::iterator mP = mPockets.begin();
		while(mP != mPockets.end())
		{
            for(size_t i = 0; i < (*mP)->PhysicsPlanes().size(); ++i)
			{
				int key = pe->AddRigidBodyBoundedPlane((*mP)->PhysicsPlanes()[i]);
				(*mP)->AddPlane(key);
				pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propCollidable, true);
				pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propActive, true);
				pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propTranslatable, false);
				pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propUseGravity, false);
			}
			++mP;
		}
	//}
	//// setup AI physics
	Physics::Engine * AI_pe = Game::Get()->GetAIPhysics();
	{
		std::vector< Geometry::Plane3D >::iterator it = mWalls.begin();
		while(it != mWalls.end())
		{
			int key = AI_pe->AddRigidBodyPlane(*it);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propCollidable, true);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propActive, true);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propTranslatable, false);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propUseGravity, false);
			++it;
		}
		std::vector< Physics::BoundedPlane >::iterator it3 = mPolys.begin();
		while(it3 != mPolys.end())
		{
			int key = AI_pe->AddRigidBodyBoundedPlane(*it3);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propCollidable, true);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propActive, true);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propTranslatable, false);
			AI_pe->RigidBodyBool(key, Physics::Engine::eRigidBodyBool::propUseGravity, false);
			++it3;
		}
		/*std::vector< int >::iterator it2 = mAIBalls.begin();
		while(it2 != mAIBalls.end())
		{
			(*it2) = AI_pe->AddRigidBodySphere(1.0);
			AI_pe->RigidBodyBool((*it2), Physics::Engine::eRigidBodyBool::propCollidable, true);
			AI_pe->RigidBodyBool((*it2), Physics::Engine::eRigidBodyBool::propActive, true);
			AI_pe->RigidBodyBool((*it2), Physics::Engine::eRigidBodyBool::propTranslatable, true);
			AI_pe->RigidBodyBool((*it2), Physics::Engine::eRigidBodyBool::propUseGravity, true);
			AI_pe->RigidBodyScalar((*it2), Physics::Engine::eRigidBodyScalar::propMass, 1.0);
			++it2;
		}*/
		std::vector< Pocket* >::iterator mP = mPockets.begin();
		while(mP != mPockets.end())
		{
			for(size_t i = 0; i < (*mP)->PhysicsPlanes().size(); ++i)
			{
				(*mP)->AddPlane(AI_pe->AddRigidBodyBoundedPlane((*mP)->PhysicsPlanes()[i]));
			}
			++mP;
		}
	}
}

/*!
 @return The number of walls in the simulation.
*//*__________________________________________________________________________*/
size_t Playfield::WallCount(void)
{
	return mWalls.size();
}

Geometry::Vector3D Playfield::GetBallPosition(Physics::Engine *engine, int ball_number)
{
	return	(engine->RigidBodyVector3D(GetBallByNumber(ball_number)->ID(),Physics::Engine::propPosition));
}
void Playfield::RackBalls(eGameType t)
{
    int levels = 0;

	if(t == EIGHTEEN_BALL)
	{
		levels     = 5;
		//const int  ball_count = BallCount(levels);
		
		Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[0]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, -25));
		
		Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[1]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, 25));
		Geometry::Vector3D org(0, 0, 25);
		balls.clear();
		for(int i = 1; i <= levels; ++i)
		{
			org = BuildLayer(i, org);
		}
		for(unsigned int i = 0; i < mBalls.size() - 1; ++i)
		{
			Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[i + 1]->ID(), Physics::Engine::eRigidBodyVector::propPosition, balls[i]);
			Game::Get()->GetPhysics()->RigidBodyBool(mBalls[i + 1]->ID(), Physics::Engine::eRigidBodyBool::propSpinnable, true);
		}
		
	}
	else if(t == NINETEEN_BALL)
	{
        float dz = sqrt(3.f);
        float z  = 25;
		Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[0]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, -25));
		
        // front ball
		Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[1]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, z));
		
        z += dz;
        // next layer
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[2]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-1, 1, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[3]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(1, 1, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[4]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-1, -1, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[5]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(1, -1, z));

        z += dz;
        // middle layer
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[6]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-2, 2, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[7]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 2, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[8]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(2, 2, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[9]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-2, 0, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[19]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[10]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(2, 0, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[11]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-2, -2, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[12]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, -2, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[13]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(2, -2, z));

        z += dz;
        // back layer
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[14]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-1, 1, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[15]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(1, 1, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[16]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(-1, -1, z));
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[17]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(1, -1, z));

        z += dz;
        // back ball
        Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[18]->ID(), Physics::Engine::eRigidBodyVector::propPosition, Geometry::Vector3D(0, 0, z));

		balls.clear();
		

        
		for(unsigned int i = 0; i < mBalls.size() - 1; ++i)
		{
			//Game::Get()->GetPhysics()->RigidBodyVector3D(mBalls[i + 1]->ID(), Physics::Engine::eRigidBodyVector::propPosition, balls[i]);
			Game::Get()->GetPhysics()->RigidBodyBool(mBalls[i + 1]->ID(), Physics::Engine::eRigidBodyBool::propSpinnable, true);
		}
		
	}
}
void Playfield::LoadBalls(eGameType t)
{
	int ct = 0;

	if(t == EIGHTEEN_BALL)
        ct = BallCount(5);
	else if(t == NINETEEN_BALL)
		ct = 19;

    // if the balls exist, delete them
	if(mBalls.size() != 0)
	{
		for(unsigned int i = 0; i < mBalls.size(); ++i)
		{
			if(mBalls[i])
			{
				Game::Get()->GetPhysics()->RemoveRigidBody(mBalls[i]->ID());			
				delete mBalls[i];
				mBalls[i] = 0;
			}
		}
		mBalls.clear();
	}

	mGhostBall = new Ball(mRenderer, "data/ghost.png", -1);
	mGhostBall->SphereMesh()->UseShader(kSphereGhostballSh);
	for(int i = 0; i <= ct; ++i)
	{
		std::stringstream filename;
		filename << "data/balls/default/" << i << ".png";
		AddBall(filename.str(), Geometry::Vector3D(), i);
	}
}
