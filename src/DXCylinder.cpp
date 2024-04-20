/*! ========================================================================

      @file    DXCylinder.h
      @author  jmp
      @brief   Implementation of DXCylinder.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "DXCylinder.h"

#include "nsl_random.h"
#include "nsl_tools_string.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

LPD3DXEFFECT                  DXCylinder::smShader           = 0;
LPDIRECT3DVERTEXDECLARATION9  DXCylinder::smDecl             = 0;
LPDIRECT3DCUBETEXTURE9        DXCylinder::smCubemap          = 0;
LPDIRECT3DSTATEBLOCK9         DXCylinder::smRenderStateBlock = 0;
bool                          DXCylinder::smInRenderState    = false;
bool                          DXCylinder::smValidSharedData  = false;


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  // shader stream declaration
  D3DVERTEXELEMENT9 gDeclShell[] =
  {
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
    D3DDECL_END()
  };
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
DXCylinder::DXCylinder(Graphics::Renderer *renderer,float radius,float length,unsigned int r,unsigned int s)
/*! Constructor.

    @param renderer  Renderer to attach to.
    @param tex       Name of the sphere's texture file (cannot be empty).
    @param r         Number of rings in the sphere (vertical slices).
    @param s         Number of segments in the sphere (horizontal slices).
*/
: Graphics::Primitive(renderer),
  mRadius(radius),mLength(length),
  mRes(r),mSegments(s),mShaderTech("RailV11P11"),
  mVtxBuffer(0),mTexture(0)
{
  mNumVerts = (mSegments + 1) * mRes;	
  mNumIndxs = 6 * mRes * mSegments;
  mNumPolys = 2 * mRes * mSegments;
  
	SetTranslation(0.0f,0.0f,0.0f);
	SetRotation(0.0f,0.0f,0.0f,1.0f);
	SetScale(1.0f,1.0f,1.0f);
	
	RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
DXCylinder::~DXCylinder(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void DXCylinder::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();
	
	// Create buffers.
	ENFORCE_DX(dev->CreateVertexBuffer(mNumVerts * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mVtxBuffer, 0))
	          ("Failed to create sphere vertex buffer.");
	ENFORCE_DX(dev->CreateIndexBuffer(mNumIndxs * sizeof(WORD),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&mIdxBuffer,0))
	          ("Failed to create sphere index buffer.");
  nFillBuffers();
  
  // Create shared data.
	if(!smValidSharedData)
	{
	  // Stream declaration.
    ENFORCE_DX(dev->CreateVertexDeclaration(gDeclShell,&smDecl))
              ("Failed to create sphere stream declaration.");
	  
	  // Environment map (cube texture).	  
    ENFORCE_DX(D3DXCreateCubeTextureFromFile(dev,nsl::stformat("data/skybox/%s/cube.dds",kSkyStates[Skybox::GetSkyState()].c_str()).c_str(),&smCubemap))
              ("Failed to load sphere environment map.");
    
	  // Shader.
	  nCompileShader("data/shaders/rail.fx",smShader);
	  
	  // Render state block.
		dev->BeginStateBlock();
    
		dev->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
		dev->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);
		dev->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_CURRENT);
		
		dev->SetRenderState(D3DRS_LIGHTING,TRUE);
		dev->SetRenderState(D3DRS_SPECULARENABLE,TRUE);
		dev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,D3DMCS_MATERIAL);
		dev->SetRenderState(D3DRS_SPECULARMATERIALSOURCE,D3DMCS_MATERIAL);
    dev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

		dev->EndStateBlock(&smRenderStateBlock);

    // Only do this once per restore.
		smValidSharedData = true;
	}
}

/*  ________________________________________________________________________ */
void DXCylinder::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  if(smValidSharedData)
  { 
    SAFE_RELEASE(smShader);
    SAFE_RELEASE(smDecl);
    SAFE_RELEASE(smCubemap);
    SAFE_RELEASE(smRenderStateBlock);
    smValidSharedData = false;
    smInRenderState   = false;
  }
  SAFE_RELEASE(mIdxBuffer);
  SAFE_RELEASE(mVtxBuffer);
}

/*  ________________________________________________________________________ */
void DXCylinder::EnterRenderState(void)
/*! Enter batch render state.
*/
{
  ASSERT(!smInRenderState);
	smRenderStateBlock->Apply();
  smInRenderState = true;
}

/*  ________________________________________________________________________ */
void DXCylinder::LeaveRenderState(void)
/*! Leave batch render state.
*/
{
  ASSERT(smInRenderState);
  smInRenderState = false;
}

/*  ________________________________________________________________________ */
void DXCylinder::Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &/*eyeVec*/)
/*! Render the sphere.

    @param eyePos  The location of the eye, in world space.
    @param eyeVec  The direction of the eye.
*/
{ 
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();
  
  // Enter render mode.
  if(!smInRenderState)
	  smRenderStateBlock->Apply();
	
	// Set specific options.
	dev->SetIndices(mIdxBuffer);
	dev->SetVertexDeclaration(smDecl);
	dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));

  // Set up matrices.
D3DXMATRIX  world;
D3DXMATRIX  view;
D3DXMATRIX  proj;

  world = mMatRotate * mMatTranslate * mMatScale;
  dev->SetTransform(D3DTS_WORLD,&world);
  dev->GetTransform(D3DTS_VIEW,&view);
  dev->GetTransform(D3DTS_PROJECTION,&proj);

D3DXMATRIX   vp = view * proj;
D3DXMATRIX   wn;
D3DXVECTOR4  cam(eyePos.x,eyePos.y,eyePos.z,1.0f);
D3DXVECTOR3  sunVec = Skybox::GetSunVector();
D3DXVECTOR4  sun(sunVec.x,sunVec.y,sunVec.z,1.0f);

  // Compute normal transform matrix.
  D3DXMatrixTranspose(&wn,&world);
  D3DXMatrixInverse(&wn,0,&wn);
  
  smShader->SetTechnique(mShaderTech.c_str());
  smShader->SetMatrix("gMatW",&world);
  smShader->SetMatrix("gMatWN",&wn);
  smShader->SetMatrix("gMatVP",&vp);
  smShader->SetVector("gEye",&cam);
  smShader->SetVector("gLight",&sun);
  smShader->SetTexture("gEnvirons",smCubemap);
    
      
UINT  totPasses = 1;  // In case shaders aren't used.

  // Now render.
  smShader->Begin(&totPasses,0);
  for(UINT pass = 0; pass < totPasses; pass++)
  {
    D3D_SHADERPASS_BEGIN(smShader,pass);
    dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,mNumVerts,0,mNumPolys);
    D3D_SHADERPASS_END(smShader);
  }
  smShader->End();
}

/*  ________________________________________________________________________ */
void DXCylinder::SetTranslation(float x,float y,float z)
/*! Set sphere translation.

    @param x  Translation along the X axis.
    @param y  Translation along the Y axis.
    @param z  Translation along the Z axis.
*/
{
  D3DXMatrixTranslation(&mMatTranslate,x,y,z);
} 

/*  ________________________________________________________________________ */
void DXCylinder::SetRotation(float x,float y,float z,float w)
/*! Set sphere rotation.

    @param x  Rotation quaternion X component.
    @param y  Rotation quaternion Y component.
    @param z  Rotation quaternion Z component.
    @param w  Rotation quaternion W component.
*/
{
D3DXQUATERNION  q(x,y,z,w);

  D3DXMatrixRotationQuaternion(&mMatRotate,&q);
} 

/*  ________________________________________________________________________ */
void DXCylinder::SetScale(float x,float y,float z)
/*! Set sphere scale.

    @param x  Scale along the X axis.
    @param y  Scale along the Y axis.
    @param z  Scale along the Z axis.
*/
{
  D3DXMatrixScaling(&mMatScale,x,y,z);
} 

/*  ________________________________________________________________________ */
void DXCylinder::SetTexture(const std::string &tex)
/*! Set the rectangles texture.

    @param tex  The name of the texture.
*/
{
  mTextureName = tex;
  SAFE_RELEASE(mTexture);
  mTexture = mRenderer->GetTexture(mTextureName);
  if(0 != mTexture)
    mTexture->AddRef();
}

/*  ________________________________________________________________________ */
void DXCylinder::nFillBuffers(void)
/*! Generate the vertices of the cylinder.
*/
{
Vertex       *vtx;
WORD         *idx; 
float         step = 2.0f * D3DX_PI / static_cast< float >(mRes); 
float         time = 2.0f * D3DX_PI;
float         sl   = mLength / static_cast< float >(mSegments);
float         z    = -mLength / 2.0f;

  // Lock buffers.
  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock sphere cylinder buffer.");
  ENFORCE_DX(mIdxBuffer->Lock(0,0,reinterpret_cast< void** >(&idx),0))
            ("Failed to lock cylinder index buffer.");
            
  // Generate (mSegments + 1) circles along the Z axis.
  for(unsigned int i = 0; i < mSegments + 1; ++i)
  {
    // Each segment is mRes vertices in a circle.
    // Generate the remaining vertices.
    for(unsigned int j = 0; j < mRes; ++j)
    {
    float        x = mRadius * ::cos(time);
    float        y = mRadius * ::sin(time);
    D3DXVECTOR3  n = D3DXVECTOR3(x,y,0.0f);
    
      D3DXVec3Normalize(&n,&n);
    
      vtx->pos  = D3DXVECTOR3(x,y,z);
      vtx->norm = n;
      ++vtx;
      
      time -= step;
    }
    
    z += sl;
  }
  
  // Generate indices.
int  vi = 0;

  for(unsigned int i = 0; i < mSegments; ++i)
  {
    for(unsigned int j = 0; j < mRes; ++j)
    {
      *idx++ = static_cast<WORD>(vi);
      *idx++ = static_cast<WORD>(vi + mRes);
      *idx++ = static_cast<WORD>((vi + 1) % mRes + mRes + (i * mRes));
      *idx++ = static_cast<WORD>(vi);
      *idx++ = static_cast<WORD>((vi + 1) % mRes + mRes + (i * mRes));
      *idx++ = static_cast<WORD>((vi + 1) % mRes + (i * mRes));
      ++vi;
    }
  }
 
  // Unlock buffers.
  ENFORCE_DX(mIdxBuffer->Unlock())
            ("Failed to unlock index buffer.");
  ENFORCE_DX(mVtxBuffer->Unlock())
            ("Failed to unlock vertex buffer.");
}
