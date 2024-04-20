/*!
	@file	PlayfieldPocket.cpp
	@author	Scott Smith
	@date	June 21, 2004

	@brief	Implementation of pocket class.

 (c) 2004 DigiPen (USA) Corporation, all rights reserved.
 *//*__________________________________________________________________________*/
    
#include "main.h"

#include "PlayfieldPocket.h"

/*! Constructor.
 @param renderer  The renderer to attach to.
*//*__________________________________________________________________________*/
Pocket::Pocket(Graphics::Renderer *renderer,const D3DXVECTOR3 &corner,float size)
: Graphics::Primitive(renderer),
  mCornerPoint(corner),mSize(size),isLit(false)
{
  // We must make the pocket before we restore objects, since
  // MakePocket() sets up the points needed for device restore.
  MakePocket();
  RestoreDeviceObjects();
}

/*! Destructor.
*//*__________________________________________________________________________*/
Pocket::~Pocket(void)
{
  ReleaseDeviceObjects();
}

/*! Restore D3D device objects.
*//*__________________________________________________________________________*/
void Pocket::RestoreDeviceObjects(void)
{
Vertex *vtx = 0;
Vertex *vtxLit = 0;

  ENFORCE_DX(mRenderer->GetDevice()->CreateVertexBuffer(3 * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mVtxBuffer,0))
            ("Failed to create pocket vertex buffer.");
  ENFORCE_DX(mRenderer->GetDevice()->CreateVertexBuffer(3 * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mLitVtxBuffer,0))
            ("Failed to create pocket vertex buffer.");
  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock pocket vertex buffer.");
  ENFORCE_DX(mLitVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtxLit),0))
            ("Failed to lock pocket vertex buffer.");
            
  vtx->pos      = mCornerX;
  vtx->color    = 0xCC100010;
  vtxLit->pos   = mCornerX;
  vtxLit->color = 0x10100010;
  ++vtx,++vtxLit;
  vtx->pos      = mCornerY;
  vtx->color    = 0xCC100010;
  vtxLit->pos   = mCornerY;
  vtxLit->color = 0x10100010;
  ++vtx,++vtxLit;
  vtx->pos      = mCornerZ;
  vtx->color    = 0xCC100010;
  vtxLit->pos   = mCornerZ;
  vtxLit->color = 0x10100010;

  ENFORCE_DX(mVtxBuffer->Unlock())
            ("Failed to unlock pocket vertex buffer.");
  ENFORCE_DX(mLitVtxBuffer->Unlock())
            ("Failed to unlock pocket vertex buffer.");
}

/*! Release D3D device objects.
*//*__________________________________________________________________________*/
void Pocket::ReleaseDeviceObjects(void)
{
  SAFE_RELEASE(mVtxBuffer);
  SAFE_RELEASE(mLitVtxBuffer);
}

/*  ________________________________________________________________________ */
void Pocket::Render(const D3DXVECTOR3 &/*eyePos*/,const D3DXVECTOR3 &/*eyeVec*/)
/*! Render the pocket.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  dev->SetFVF(Vertex::Format);
  if(isLit)
    dev->SetStreamSource(0,mLitVtxBuffer,0,sizeof(Vertex));
  else
    dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));

D3DXMATRIX mat;

  D3DXMatrixIdentity(&mat);

  dev->SetTransform(D3DTS_WORLD,&mat);

  dev->SetRenderState(D3DRS_ZENABLE,TRUE);
  dev->SetRenderState(D3DRS_LIGHTING,FALSE);
  dev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
  dev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
  dev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
  dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);

  dev->DrawPrimitive(D3DPT_TRIANGLELIST,0,1);
}

/*! Add the pocket to the physics engine.
*//*__________________________________________________________________________*/
void Pocket::MakePocket(void)
{
D3DXVECTOR3  x(1.0f,0.0f,0.0f);
D3DXVECTOR3  y(0.0f,1.0f,0.0f);
D3DXVECTOR3  z(0.0f,0.0f,1.0f);

  // Scale by pocket size.
	x *= mSize;
	y *= mSize;
	z *= mSize;
	
	// Orient properly based on corner position.
	if(mCornerPoint.x > 0)
		x = -x;
	if(mCornerPoint.y > 0)
		y = -y;
	if(mCornerPoint.z > 0)
		z = -z;
	mCornerX = (mCornerPoint + x);
	mCornerY = (mCornerPoint + y);
	mCornerZ = (mCornerPoint + z);

  // Figure out what planes this pocket touches.
Geometry::Point3D  p_x(mCornerPoint.x + x.x,mCornerPoint.y + x.y,mCornerPoint.z + x.z);
Geometry::Point3D  p_y(mCornerPoint.x + y.x,mCornerPoint.y + y.y,mCornerPoint.z + y.z);
Geometry::Point3D  p_z(mCornerPoint.x + z.x,mCornerPoint.y + z.y,mCornerPoint.z + z.z);

	// Back / top / left.
	if(mCornerPoint.x < 0.0f && mCornerPoint.y > 0.0f && mCornerPoint.z > 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_z,p_y,p_x));			

	// Back / top / right.
	if(mCornerPoint.x > 0.0f && mCornerPoint.y > 0.0f && mCornerPoint.z > 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_x,p_y,p_z));
	
	// Back / bottom / left.
	if(mCornerPoint.x < 0.0f && mCornerPoint.y < 0.0f && mCornerPoint.z > 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_z,p_x,p_y));

	// Back / bottom / right.
	if(mCornerPoint.x > 0.0f && mCornerPoint.y < 0.0f && mCornerPoint.z > 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_x,p_z,p_y));
	
	// Front / top / left.
	if(mCornerPoint.x > 0.0f && mCornerPoint.y > 0.0f && mCornerPoint.z < 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_z,p_y,p_x));

	// Front / top / right.
	if(mCornerPoint.x < 0.0f && mCornerPoint.y > 0.0f && mCornerPoint.z < 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_x,p_y,p_z));
	
	// Front / bottom / left.
	if(mCornerPoint.x > 0.0f && mCornerPoint.y < 0.0f && mCornerPoint.z < 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_z,p_x,p_y));
	
	// Front / bottom / right.
	if(mCornerPoint.x < 0.0f && mCornerPoint.y < 0.0f && mCornerPoint.z < 0.0f)
		mPhysicsPlanes.push_back(Physics::BoundedPlane(p_x,p_z,p_y));
}