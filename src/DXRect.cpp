/*! ========================================================================

      @file    DXRect.cpp
      @author  jmp
      @brief   Implementation of DXRect.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "DXRect.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

LPD3DXEFFECT                  DXRect::smShader           = 0;
LPDIRECT3DVERTEXDECLARATION9  DXRect::smDecl             = 0;
LPDIRECT3DSTATEBLOCK9         DXRect::smRenderStateBlock = 0;
bool                          DXRect::smInRenderState    = false;
bool                          DXRect::smSharedValid      = false;

namespace
{
  // shader stream declaration
  D3DVERTEXELEMENT9 gDeclShell[] =
  {
    { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0},
    { 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
  };
}

/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  // counts
  const int  kNumVertices  = 4;
  const int  kNumTriangles = 2;
  
  // color indices
  const int  kTopLeft     = 0;
  const int  kTopRight    = 1;
  const int  kBottomLeft  = 2;
  const int  kBottomRight = 3;
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
DXRect::DXRect(Graphics::Renderer *renderer,DWORD tl,DWORD tr,DWORD bl,DWORD br,const std::string &tex)
/*! Constructor.
    
    @param renderer  Renderer to attach to.
    @param tl        Top-left color.
    @param tr        Top-right color.
    @param bl        Bottom-left color.
    @param br        Bottom-right color.
    @param 
*/
: Graphics::Primitive(renderer),
  mTexture(0),mVtxBuffer(0),
  mTextureName(tex)
{
  mColors[kTopLeft]     = tl;
  mColors[kTopRight]    = tr;
  mColors[kBottomLeft]  = bl;
  mColors[kBottomRight] = br;
	RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
DXRect::~DXRect(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void DXRect::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{	
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  // Get the texture. Add a reference to it so we don't
  // release the renderer's cached copy when we die.
  mTexture = mRenderer->GetTexture(mTextureName);
  if(0 != mTexture)
    mTexture->AddRef();

	// Create vertex buffer.
	ENFORCE_DX(dev->CreateVertexBuffer(kNumVertices * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mVtxBuffer, 0))
	          ("Failed to create rectangle vertex buffer.");

  // Lock it and fill it.
Vertex* vtx = 0;
  
  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock rectangle vertex buffer.");

  // Left, bottom.
  vtx->pos = D3DXVECTOR3(-0.5f,-0.5f,1.0f);
  vtx->c   = mColors[kBottomLeft];
  vtx->u   = 0.0f;
  vtx->v   = 1.0f;
  ++vtx;
  
  // Left, top.
  vtx->pos = D3DXVECTOR3(-0.5f, 0.5f, 1.0f);
  vtx->c   = mColors[kTopLeft];
  vtx->u   = 0.0f;
  vtx->v   = 0.0f;
  ++vtx;
  
  // Right, bottom.
  vtx->pos = D3DXVECTOR3( 0.5f, -0.5f, 1.0f);
  vtx->c   = mColors[kBottomRight];
  vtx->u   = 1.0f;
  vtx->v   = 1.0f;
  ++vtx;
  
  // Right, top.
  vtx->pos = D3DXVECTOR3( 0.5f, 0.5f, 1.0f);
  vtx->c   = mColors[kTopRight];
  vtx->u   = 1.0f;
  vtx->v   = 0.0f;
	
	ENFORCE_DX(mVtxBuffer->Unlock())
	          ("Failed to unlock rectangle vertex buffer.");

  // Create the state block for rendering.
  if(!smSharedValid)
  {
    // Stream declaration.
    ENFORCE_DX(dev->CreateVertexDeclaration(gDeclShell,&smDecl))
              ("Failed to create rectangle stream declaration.");
    
    // Shader.
    nCompileShader("data/shaders/ui.fx",smShader);
  
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
void DXRect::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  if(smSharedValid)
  {
    SAFE_RELEASE(smShader);
    SAFE_RELEASE(smDecl);
    SAFE_RELEASE(smRenderStateBlock);
    smSharedValid = false;
    smInRenderState = false;
  }
  SAFE_RELEASE(mVtxBuffer);
  SAFE_RELEASE(mTexture);
}

/*  ________________________________________________________________________ */
void DXRect::EnterRenderState(void)
/*! Enter batch render state.
*/
{
  ASSERT(!smInRenderState);
	smRenderStateBlock->Apply();
  smInRenderState = true;
}

/*  ________________________________________________________________________ */
void DXRect::LeaveRenderState(void)
/*! Leave batch render state.
*/
{
  ASSERT(smInRenderState);
  smInRenderState = false;
}

/*  ________________________________________________________________________ */
void DXRect::SetTexture(const std::string &tex)
/*! Set the rectangles texture.

    @param tex  The name of the texture.
*/
{
  mTextureName = tex;
  SAFE_RELEASE(mTexture);
  mTexture = mRenderer->GetTexture(tex);
  if(0 != mTexture)
    mTexture->AddRef();
}

/*  ________________________________________________________________________ */
void DXRect::DrawRect(int x,int y,int w,int h)
/*! Render a rectangle to the screen.

    @param x    X coordinate of the rectangle.
    @param y    Y coordinate of the rectangle.
    @param w    Width of the rectangle.
    @param h    Height of the rectangle.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  // Enter render mode.
  if(!smInRenderState)
	  smRenderStateBlock->Apply();
  
  dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));
	dev->SetVertexDeclaration(smDecl);
	
D3DXMATRIX  matTranslate;
D3DXMATRIX  matScale;
D3DXMATRIX  view;
D3DXMATRIX  proj;

	// Create a matrix to transform the rectangle to the desired
	// position and size.
	D3DXMatrixTranslation(&matTranslate,x + (w / 2.0f),-y - (h / 2.0f),0.0f);
	D3DXMatrixScaling(&matScale,static_cast< float >(w),static_cast< float >(h),0.0f);
	D3DXMatrixMultiply(&matTranslate,&matScale,&matTranslate);

  // Set shader properties.
	dev->SetTransform(D3DTS_WORLD,&matTranslate);
  dev->GetTransform(D3DTS_VIEW,&view);
  dev->GetTransform(D3DTS_PROJECTION,&proj);

D3DXMATRIX  wvp = matTranslate * view * proj;

  smShader->SetMatrix("gMatWVP",&wvp);
  smShader->SetTexture("gTexture",mTexture);

  // Select appropriate technique.
  if(mTexture == 0)
    smShader->SetTechnique("UIColorV11P11");
  else
    smShader->SetTechnique("UITextureV11P11");
  
  // Render.
UINT  totPasses = 0;

  smShader->Begin(&totPasses,0);
  for(UINT pass = 0; pass < totPasses; ++pass)
  {
    D3D_SHADERPASS_BEGIN(smShader,pass);
	  dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,kNumTriangles);
    D3D_SHADERPASS_END(smShader);
  }
  smShader->End();
}
