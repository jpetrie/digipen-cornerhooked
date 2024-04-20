/*! ========================================================================

      @file    DXCircle.cpp
      @author  jmp
      @brief   Implementation of DXCircle.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "DXCircle.h"

#include "Quaternion.h"

/*                                                                 variables
---------------------------------------------------------------------------- */

LPDIRECT3DSTATEBLOCK9  DXCircle::smRenderStateBlock = 0;
bool                   DXCircle::smInRenderState    = false;
bool                   DXCircle::smSharedValid      = false;

/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  // counts
  const int  kNumVertices  = 2;
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
DXCircle::DXCircle(Graphics::Renderer *renderer,DWORD color,unsigned int resolution)
/*! Constructor.
    
    @param renderer    Renderer to attach to.
    @param color       Circle color.
    @param resolution  Resolution (smoothness) of the circle.
*/
: Graphics::Primitive(renderer),
  mVtxBuffer(0),
  mColor(color),mResolution(resolution)
{
	RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
DXCircle::~DXCircle(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void DXCircle::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{	
LPDIRECT3DDEVICE9  dev  = mRenderer->GetDevice();
Vertex            *vtx  = 0;
float              step = 2.0f * D3DX_PI / static_cast< float >(mResolution); 
float              time = 2.0f * D3DX_PI;

  // Create vertex buffer.
	ENFORCE_DX(dev->CreateVertexBuffer((mResolution + 2) * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mVtxBuffer, 0))
	          ("Failed to create circle vertex buffer.");

  // Lock the buffer and write the coordinates into it.
  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock circle vertex buffer.");
  
  // Generate the first vertex.
  vtx->pos = D3DXVECTOR3(0.0f,0.0f,0.0f); 
  vtx->c   = mColor;
  ++vtx;
  
  // Generate the remaining vertices.
  for(unsigned int i = 0; i < mResolution; ++i)
  {
  float  x = cos(time);
  float  y = sin(time);
  
    vtx->pos = D3DXVECTOR3(x,y,0.0f);
    vtx->c   = mColor;
    ++vtx;
    
    time -= step;
  }
  
  // Insert the first vertex again.
  vtx->pos = D3DXVECTOR3(cos(2.0f * D3DX_PI),sin(2.0f * D3DX_PI),0.0f);
  vtx->c   = mColor;
  
  // Unlock the buffer.
  ENFORCE_DX(mVtxBuffer->Unlock())
	          ("Failed to unlock circle vertex buffer.");

  // Create the state block for rendering.
  if(!smSharedValid)
  {  
    dev->BeginStateBlock();

	  dev->SetRenderState(D3DRS_LIGHTING,FALSE);
    dev->SetRenderState(D3DRS_ZENABLE,TRUE);

    dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	  dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	  dev->SetRenderState(D3DRS_ALPHATESTENABLE,TRUE);
	  dev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);   
	  dev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

    dev->EndStateBlock(&smRenderStateBlock);
    
    // Shared data is safe now.
    smSharedValid = true;
  }
}

/*  ________________________________________________________________________ */
void DXCircle::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  if(smSharedValid)
  {
    SAFE_RELEASE(smRenderStateBlock);
    smSharedValid = false;
    smInRenderState = false;
  }
  SAFE_RELEASE(mVtxBuffer);
}

/*  ________________________________________________________________________ */
void DXCircle::EnterRenderState(void)
/*! Enter batch render state.
*/
{
  ASSERT(!smInRenderState);
	smRenderStateBlock->Apply();
  smInRenderState = true;
}

/*  ________________________________________________________________________ */
void DXCircle::LeaveRenderState(void)
/*! Leave batch render state.
*/
{
  ASSERT(smInRenderState);
  smInRenderState = false;
}

/*  ________________________________________________________________________ */
void DXCircle::DrawCircle(float x,float y,float z,float r,D3DXVECTOR3 normal)
/*! Render a circle to the screen.

    @param x       Center X coordinate.
    @param y       Center Y coordinate.
    @param z       Center Z coordinate.
    @param r       Radius.
    @param normal  Circle face normal.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();


  D3DXVec3Normalize(&normal,&normal);

Quaternion  q(1.0f,normal.x,normal.y,normal.z);

  q.Normalize();

Geometry::Matrix4x4   basis = q.Basis();

  // Enter render mode.
  if(!smInRenderState)
	  smRenderStateBlock->Apply();

  dev->SetPixelShader(0);
  dev->SetVertexShader(0);
  dev->SetFVF(Vertex::Format);
  dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));

D3DXMATRIX  trans;
D3DXMATRIX  scale;
D3DXMATRIX  rot(basis[0][0],basis[0][1],basis[0][2],basis[0][3],
                basis[1][0],basis[1][1],basis[1][2],basis[1][3],
                basis[2][0],basis[2][1],basis[2][2],basis[2][3],
                basis[3][0],basis[3][1],basis[3][2],basis[3][3]);

 // D3DXMatrixTranspose(&rot,&rot);

  D3DXMatrixTranslation(&trans,x,y,z);
  D3DXMatrixScaling(&scale,r,r,r);
  trans = rot;
  
  dev->SetTransform(D3DTS_WORLD,&trans);

  // Render.
	dev->DrawPrimitive(D3DPT_TRIANGLEFAN,0,mResolution);
}
