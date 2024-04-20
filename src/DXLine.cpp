/*! ========================================================================

      @file    DXLine.cpp
      @author  jmp
      @brief   Implementation of DXLine.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "DXLine.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

LPDIRECT3DSTATEBLOCK9  DXLine::smRenderStateBlock = 0;
bool                   DXLine::smInRenderState    = false;
bool                   DXLine::smSharedValid      = false;

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
DXLine::DXLine(Graphics::Renderer *renderer,DWORD start,DWORD stop)
/*! Constructor.
    
    @param renderer   Renderer to attach to.
    @param start      Starting point color.
    @param stop       Ending point color.
*/
: Graphics::Primitive(renderer),
  mVtxBuffer(0),
  mColor0(start),mColor1(stop)
{
	RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
DXLine::~DXLine(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void DXLine::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{	
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  // Create vertex buffer.
	ENFORCE_DX(dev->CreateVertexBuffer(kNumVertices * sizeof(Vertex),D3DUSAGE_DYNAMIC,0,D3DPOOL_DEFAULT,&mVtxBuffer, 0))
	          ("Failed to create line vertex buffer.");

  // Create the state block for rendering.
  if(!smSharedValid)
  {  
    dev->BeginStateBlock();
	  dev->SetRenderState(D3DRS_LIGHTING,FALSE);
    dev->SetRenderState(D3DRS_ZENABLE,TRUE);
    dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	  dev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);   
	  dev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
    dev->EndStateBlock(&smRenderStateBlock);
    
    // Shared data is safe now.
    smSharedValid = true;
  }
}

/*  ________________________________________________________________________ */
void DXLine::ReleaseDeviceObjects(void)
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
void DXLine::EnterRenderState(void)
/*! Enter batch render state.
*/
{
  ASSERT(!smInRenderState);
	smRenderStateBlock->Apply();
  smInRenderState = true;
}

/*  ________________________________________________________________________ */
void DXLine::LeaveRenderState(void)
/*! Leave batch render state.
*/
{
  ASSERT(smInRenderState);
  smInRenderState = false;
}

/*  ________________________________________________________________________ */
void DXLine::DrawLine(float x0,float y0,float z0,float x1,float y1,float z1)
/*! Render a line to the screen.

    @param x0  Starting X coordinate.
    @param y0  Starting Y coordinate.
    @param z0  Starting Z coordinate.
    @param x1  Ending X coordinate.
    @param y1  Ending Y coordinate.
    @param z1  Ending Z coordinate.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();
Vertex            *vtx = 0;

  // Enter render mode.
  if(!smInRenderState)
	  smRenderStateBlock->Apply();

  // Lock the buffer and write the coordinates into it.
  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock line vertex buffer.");
  
  vtx->pos = D3DXVECTOR3(x0,y0,z0);
  vtx->c   = mColor0;
  vtx++;
  vtx->pos = D3DXVECTOR3(x1,y1,z1);
  vtx->c   = mColor1;
  
  ENFORCE_DX(mVtxBuffer->Unlock())
	          ("Failed to unlock line vertex buffer.");

D3DXMATRIX world;
  
  D3DXMatrixIdentity(&world);
  
  dev->SetFVF(Vertex::Format);
  dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));
  dev->SetTransform(D3DTS_WORLD,&world);
	dev->DrawPrimitive(D3DPT_LINELIST,0,1);
}
