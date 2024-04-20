/*! ========================================================================

      @file    DXSphere.h
      @author  jmp
      @brief   Implementation of DXSphere.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include <math.h>
#include "Game.h"
#include "DXSphere.h"

#include "nsl_random.h"
#include "nsl_tools_string.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

LPD3DXEFFECT                  DXSphere::smShader           = 0;
LPDIRECT3DTEXTURE9            DXSphere::smCaustics[kSphereCausticTextures];
LPDIRECT3DVERTEXDECLARATION9  DXSphere::smDecl             = 0;
LPDIRECT3DCUBETEXTURE9        DXSphere::smCubemap          = 0;
LPDIRECT3DSTATEBLOCK9         DXSphere::smRenderStateBlock = 0;
bool                          DXSphere::smInRenderState    = false;
bool                          DXSphere::smValidSharedData  = false;


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  // shader stream declaration
  D3DVERTEXELEMENT9 gDeclShell[] =
  {
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
  };
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
DXSphere::DXSphere(Graphics::Renderer *renderer,const std::string &tex,unsigned int r,unsigned int s)
/*! Constructor.

    @param renderer  Renderer to attach to.
    @param tex       Name of the sphere's texture file (cannot be empty).
    @param r         Number of rings in the sphere (vertical slices).
    @param s         Number of segments in the sphere (horizontal slices).
*/
: Graphics::Primitive(renderer),
  mShaderTech("BallV11P11"),
  mTexture(0),mTextureName(tex)
{
static  random  rng(64345);

  for( unsigned int i = 0; i < D3DXSPHERE_LOD_LEVELS; ++i )
  {
    mVtxBuffer[i] = 0;
	mRings[i] = r;
	mSegments[i] = s;
	mNumVerts[i] = (mRings[i] + 1) * (mSegments[i] + 1);
    mNumIndxs[i] = (2 * mRings[i]) * (mSegments[i] + 1);
    mNumPolys[i] = mNumIndxs[i] - 2;
	r = static_cast<unsigned int>( std::ceil( (r*3)/4.0 ) );
	s = static_cast<unsigned int>( std::ceil( (s*3)/4.0 ) );
  }

  // Set up the default material.
D3DCOLORVALUE  mc = { 1.0f, 1.0f, 1.0f, 1.0f };
D3DCOLORVALUE  me = { 0.0f, 0.0f, 0.0f, 0.0f };
float          mp = 30.0f;
	
	SetMaterial(mc,mc,mc,me,mp);
	SetTranslation(0.0f,0.0f,0.0f);
	SetRotation(0.0f,0.0f,0.0f,1.0f);
	SetScale(1.0f,1.0f,1.0f);
	
	mCausticID = rng.rand_int(0,31);
	
	RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
DXSphere::~DXSphere(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void DXSphere::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();
	
  // Get the texture. Add a reference to it so we don't
  // release the renderer's cached copy when we die.
  mTexture = mRenderer->GetTexture(mTextureName);
  if(0 != mTexture)
    mTexture->AddRef();
	
	// Create buffers.
    for( unsigned int i = 0; i < D3DXSPHERE_LOD_LEVELS; ++i )
	{
		ENFORCE_DX(dev->CreateVertexBuffer(mNumVerts[i] * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mVtxBuffer[i], 0))
	          ("Failed to create sphere vertex buffer.");
		ENFORCE_DX(dev->CreateIndexBuffer(mNumIndxs[i] * sizeof(WORD),D3DUSAGE_WRITEONLY,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&mIdxBuffer[i],0))
	          ("Failed to create sphere index buffer.");
	  nFillBuffers(i);
	}
  
  // Create shared data.
	if(!smValidSharedData)
	{
	  // Stream declaration.
    ENFORCE_DX(dev->CreateVertexDeclaration(gDeclShell,&smDecl))
              ("Failed to create sphere stream declaration.");
	  
	  // Environment map (cube texture).	  
    ENFORCE_DX(D3DXCreateCubeTextureFromFile(dev,nsl::stformat("data/skybox/%s/cube.dds",kSkyStates[Skybox::GetSkyState()].c_str()).c_str(),&smCubemap))
              ("Failed to load sphere environment map.");

    // Caustics textures.
    for(unsigned int i = 0; i < kSphereCausticTextures; ++i)
    { 
      smCaustics[i] = mRenderer->GetTexture(nsl::stformat("data/caustics/caust%d.tga",i));
      smCaustics[i]->AddRef();
    }
    
	  // Shader.
	  nCompileShader("data/shaders/ball.fx",smShader);
	  
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
void DXSphere::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  if(smValidSharedData)
  {
    for(unsigned int i = 0; i < kSphereCausticTextures; ++i)
      SAFE_RELEASE(smCaustics[i]);
      
    SAFE_RELEASE(smShader);
    SAFE_RELEASE(smDecl);
    SAFE_RELEASE(smCubemap);
    SAFE_RELEASE(smRenderStateBlock);
    smValidSharedData = false;
    smInRenderState   = false;
  }
  for( unsigned int i = 0; i < D3DXSPHERE_LOD_LEVELS; ++i )
  {
	SAFE_RELEASE(mIdxBuffer[i]);
	SAFE_RELEASE(mVtxBuffer[i]);
  }
  SAFE_RELEASE(mTexture);
}

/*  ________________________________________________________________________ */
void DXSphere::EnterRenderState(void)
/*! Enter batch render state.
*/
{
  ASSERT(!smInRenderState);
	smRenderStateBlock->Apply();
  smInRenderState = true;
}

/*  ________________________________________________________________________ */
void DXSphere::LeaveRenderState(void)
/*! Leave batch render state.
*/
{
  ASSERT(smInRenderState);
  smInRenderState = false;
}

/*  ________________________________________________________________________ */
void DXSphere::Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &/*eyeVec*/)
/*! Render the sphere.

    @param eyePos  The location of the eye, in world space.
    @param eyeVec  The direction of the eye.
*/
{ 
	// set sphere level of detail
	static unsigned int	LOD = 0;
	const float s = ( mMatScale(0,0) + mMatScale(1,1) + mMatScale(2,2) ) / 3.0f;
	const float x = eyePos.x - mMatTranslate(3,0);
	const float y = eyePos.y - mMatTranslate(3,1);
	const float z = eyePos.z - mMatTranslate(3,2);
	const float distSqr = ( x*x + y*y + z*z ) / s;
	if( distSqr < 48.0*48.0 )
		LOD = 0;
	else if( distSqr < 80.0*80.0 )
		LOD = 1;
	else if( distSqr < 128.0*128.0 )
		LOD = 2;
	else
		LOD = 3;
	
	// make sure the LOD wasn't accidently set above maximum
	if( LOD >= D3DXSPHERE_LOD_LEVELS )
		LOD = D3DXSPHERE_LOD_LEVELS-1;

  LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  mCausticClock.Update();
  if(mCausticClock.Accumulated() > kWaterTimeStep)
  {
    mCausticID = (mCausticID + 1) % kSphereCausticTextures;
    mCausticClock.Reset();
  }
  
  // Enter render mode.
  if(!smInRenderState)
	  smRenderStateBlock->Apply();
	
	// Set specific options.
	dev->SetMaterial(&mMaterial);
	dev->SetIndices(mIdxBuffer[LOD]);
	dev->SetVertexDeclaration(smDecl);
	dev->SetStreamSource(0,mVtxBuffer[LOD],0,sizeof(Vertex));

  // Set up matrices.
D3DXMATRIX  world;
D3DXMATRIX  view;
D3DXMATRIX  proj;

  world = mMatScale * mMatRotate * mMatTranslate;
  dev->SetTransform(D3DTS_WORLD,&world);
  dev->GetTransform(D3DTS_VIEW,&view);
  dev->GetTransform(D3DTS_PROJECTION,&proj);

D3DXMATRIX   vp = view * proj;
D3DXMATRIX   wn;
D3DXVECTOR4  cam(eyePos.x,eyePos.y,eyePos.z,1.0f);
D3DXVECTOR3  sunVec = Skybox::GetSunVector();
D3DXVECTOR4  sun(sunVec.x,sunVec.y,sunVec.z,1.0f);
DWORD        sunColor = Skybox::GetSunColor();
D3DXVECTOR4  sunColorVec(((sunColor & 0x00FF0000) >> 16) / 255.0f,  // R.
                         ((sunColor & 0x0000FF00) >> 8)  / 255.0f,  // G.
                         ((sunColor & 0x000000FF) >> 0)  / 255.0f,  // B.
                         ((sunColor & 0xFF000000) >> 24) / 255.0f); // A.

  // Compute normal transform matrix.
  D3DXMatrixTranspose(&wn,&world);
  D3DXMatrixInverse(&wn,0,&wn);
  
  smShader->SetTechnique(mShaderTech.c_str());
  smShader->SetMatrix("gMatW",&world);
  smShader->SetMatrix("gMatWN",&wn);
  smShader->SetMatrix("gMatVP",&vp);
  smShader->SetVector("gEye",&cam);
  smShader->SetVector("gLight",&sun);
  smShader->SetVector("gAmbient",&sunColorVec);
  smShader->SetTexture("gTexture",mTexture);
  smShader->SetTexture("gEnvirons",smCubemap);
  smShader->SetTexture("gCaustic",smCaustics[mCausticID]);
    
      
UINT  totPasses = 1;  // In case shaders aren't used.

  // Now render.
  smShader->Begin(&totPasses,0);
  for(UINT pass = 0; pass < totPasses; pass++)
  {
    D3D_SHADERPASS_BEGIN(smShader,pass);
    dev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,0,0,mNumVerts[LOD],0,mNumPolys[LOD]);
    D3D_SHADERPASS_END(smShader);
  }
  smShader->End();
}

/*  ________________________________________________________________________ */
void DXSphere::SetTranslation(float x,float y,float z)
/*! Set sphere translation.

    @param x  Translation along the X axis.
    @param y  Translation along the Y axis.
    @param z  Translation along the Z axis.
*/
{
  D3DXMatrixTranslation(&mMatTranslate,x,y,z);
} 

/*  ________________________________________________________________________ */
void DXSphere::SetRotation(float x,float y,float z,float w)
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
void DXSphere::SetScale(float x,float y,float z)
/*! Set sphere scale.

    @param x  Scale along the X axis.
    @param y  Scale along the Y axis.
    @param z  Scale along the Z axis.
*/
{
  D3DXMatrixScaling(&mMatScale,x,y,z);
} 

/*  ________________________________________________________________________ */
void DXSphere::SetTexture(const std::string &tex)
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
void DXSphere::SetMaterial(D3DCOLORVALUE ambient,D3DCOLORVALUE diffuse,D3DCOLORVALUE specular,D3DCOLORVALUE emissive,float power)
/*! Set sphere material.

    @param ambient   Ambient color value.
    @param diffuse   Diffuse color value.
    @param specular  Specular color value.
    @param emissive  Emissive color value.
    @param power     Power of specular value (how sharp the hilight is).
*/
{
	mMaterial.Ambient  = ambient;
	mMaterial.Diffuse  = diffuse;
	mMaterial.Specular = specular;
	mMaterial.Emissive = emissive; 
	mMaterial.Power    = power;
}

/*  ________________________________________________________________________ */
void DXSphere::nFillBuffers(unsigned int i)
/*! Generate the vertices of the sphere.
*/
{
Vertex       *vtx;
WORD         *idx; 
WORD          vi;
unsigned int  cr;                                  // Current ring.
unsigned int  cs;                                  // Current segment.
float         rad = (D3DX_PI / mRings[i]);         // Ring angle delta.
float         sad = (2.0f * D3DX_PI / mSegments[i]);  // Segment angle delta.

  // assert
  ASSERT( i < D3DXSPHERE_LOD_LEVELS );

  // Start at the 1st vertex.
  vi = 0;

  // Lock buffers.
  ENFORCE_DX(mVtxBuffer[i]->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock sphere vertex buffer.");
  ENFORCE_DX(mIdxBuffer[i]->Lock(0,0,reinterpret_cast< void** >(&idx),0))
            ("Failed to lock sphere index buffer.");
            
  // Rings first...
	for(cr = 0; cr < mRings[i] + 1; cr++)
	{
	float  r0 = sinf(cr * rad);
	float  y0 = cosf(cr * rad);
    
    // ...then segments.
		for(cs = 0; cs < mSegments[i] + 1; cs++)
		{
		float  x0 = r0 * sinf(cs * sad);
		float  z0 = r0 * cosf(cs * sad);

			// Add a vertex to the sphere's triangle strip.
			vtx->pos = D3DXVECTOR3(x0,y0,z0);

			// Sphere is "unit" and centered at (0,0,0), so the normals at
			// each vertex are simply the coordinates as vectors.
			vtx->norm = vtx->pos;
			D3DXVec3Normalize(&vtx->norm,&vtx->norm);

			vtx->tex = D3DXVECTOR2(1.0f - (static_cast< float >(cs) / static_cast< float >(mSegments[i])),
			                              (static_cast< float >(cr) / static_cast< float >(mRings[i])));
			
      // Next, please.
			vtx++;
			
			// We get two indices out of this vertex, unless we're on the last ring.
			if(cr != mRings[i]) 
			{
				*idx++ = vi; 
				*idx++ = vi + static_cast< WORD >(mSegments[i] + 1); 
				vi++; 
			}
		}
	}

  // Unlock buffers.
  ENFORCE_DX(mIdxBuffer[i]->Unlock())
            ("Failed to unlock index buffer.");
  ENFORCE_DX(mVtxBuffer[i]->Unlock())
            ("Failed to unlock vertex buffer.");
}
