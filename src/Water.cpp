/*! ========================================================================

      @file    Water.cpp
      @author  jmc,jmp
      @brief   Implementation of water renderer.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Game.h"
#include "Camera.h"
#include "Skybox.h"
#include "Window.h"
#include "Water.h"
#include "UIScreen.h"
#include "nsl_random.h"
#include "nsl_tools_string.h"


/*                                                                 variables
---------------------------------------------------------------------------- */

namespace
{
  // limits
  const int  kNormalMapSz = 256;  //!< Dimension of normal map texture.
  
  // geometric wave constants
  const float  kGeoWaveAmpLimit = 0.075f;  // Max geometry wave amplitude.
  const float  kGeoWaveFrqLimit = 0.5;     // Max geometry wave frequency.
  const float  kGeoWaveSpdLimit = 3.0f;    // Max geometry wave speed.
  
  // wave simulation constants
  const float  kWaveGravity = 30.0f;
}


/*                                                                 variables
---------------------------------------------------------------------------- */

Water::GeoWaveParams          Water::smGeoWaveParams    = { 0 };
Water::TexWaveParams          Water::smTexWaveParams    = { 0 };
LPD3DXEFFECT                  Water::smShader           = 0;
LPDIRECT3DVERTEXDECLARATION9  Water::smDecl             = 0;
LPDIRECT3DVERTEXDECLARATION9  Water::smDeclRTT          = 0;
LPDIRECT3DCUBETEXTURE9        Water::smCubemap          = 0;
LPDIRECT3DTEXTURE9            Water::smCosLUT           = 0;
LPDIRECT3DTEXTURE9            Water::smNoiseLUT         = 0;
LPDIRECT3DVERTEXBUFFER9       Water::smRTTVB            = 0;
LPDIRECT3DSTATEBLOCK9         Water::smRenderStateBlock = 0;
bool                          Water::smInRenderState    = false;
bool                          Water::smValidSharedData  = false;

namespace
{
  // rng
  random  gRNG(clock());

  // shader stream declaration; water
  D3DVERTEXELEMENT9 gDeclShell[] =
  {
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
  };
  
  // shader stream declaration; water normal map
  D3DVERTEXELEMENT9 gDeclShellRTT[] =
  {
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
  };
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Water::Water(Graphics::Renderer *renderer,float sx,float sy,unsigned int rx,unsigned int ry)
/*! Constructor.

    @param renderer  The renderer to attach to.
    @param sx        Size of the plane in the X dimension.
    @param sy        Size of the plane in the Y dimension.
    @param rx        Resolution of the plane in the X dimension.
    @param ry        Resolution of the plane in the Y dimension.
*/
: Graphics::Primitive(renderer),
  mVtxBuffer(0),mIdxBuffer(0),
  mWaveSimTime(0.0f),
  mSX(sx),mSY(sy),mRX(rx),mRY(ry)
{
 ASSERT(sx > 0.0f && sy > 0.0f);
 ASSERT(rx > 0 && ry > 0);
  
  mNumVerts  = (mRX + 1) * (mRY + 1);
  mNumStrips = mRY;
  mNumIndxs  = mNumStrips * (mRX + 1) * 2;
  
  SetTranslation(0.0f,0.0f,0.0f);
  SetRotation(0.0f,0.0f,0.0f,1.0f);
  SetScale(1.0f,1.0f,1.0f);
  
  RestoreDeviceObjects();

  // Initialize waves.
  nInitGeoWaves();
  nInitTexWaves();
}

/*  ________________________________________________________________________ */
Water::~Water(void)
/*! Destructor
*/
{
  ReleaseDeviceObjects();
}

/*  ________________________________________________________________________ */
void Water::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  // Create the buffers.
  ENFORCE_DX(dev->CreateVertexBuffer(mNumVerts * sizeof(Vertex),0,0,D3DPOOL_DEFAULT,&mVtxBuffer,0))
            ("Failed to create water vertex buffer.");
  ENFORCE_DX(dev->CreateIndexBuffer(mNumIndxs * sizeof(WORD),0,D3DFMT_INDEX16,D3DPOOL_DEFAULT,&mIdxBuffer,0))
            ("Failed to create water index buffer.");
  nFillBuffers();
  
  // Create the normal map.
  ENFORCE_DX(dev->CreateTexture(kNormalMapSz,kNormalMapSz,1,D3DUSAGE_RENDERTARGET,D3DFMT_R5G6B5,D3DPOOL_DEFAULT,&mNormalMap,0))
            ("Failed to create water normal map.");
 
  // Create shared data.
  if(!smValidSharedData)
  { 
    // Stream declarations.
    ENFORCE_DX(dev->CreateVertexDeclaration(gDeclShell,&smDecl))
              ("Failed to create water stream declaration.");
    ENFORCE_DX(dev->CreateVertexDeclaration(gDeclShellRTT,&smDeclRTT))
              ("Failed to create water RTT stream declaration.");
            
    // Environment map (cube texture).
    ENFORCE_DX(D3DXCreateCubeTextureFromFile(dev,nsl::stformat("data/skybox/%s/cube.dds",kSkyStates[Skybox::GetSkyState()].c_str()).c_str(),&smCubemap))
              ("Failed to load water environment map.");
    
    // RTT vertex buffer.
    ENFORCE_DX(dev->CreateVertexBuffer(4 * sizeof(VertexRTT),0,0,D3DPOOL_DEFAULT,&smRTTVB,0))
              ("Failed to create water RTT vertex buffer.");
    nFillBuffersRTT();
    
    // Cosine lookup table texture.
    ENFORCE_DX(dev->CreateTexture(kNormalMapSz,1,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&smCosLUT,0))
              ("Failed to create water cosine LUT texture.");
    nFillCosineLUT();
    
    // Noise bias lookup texture.
    ENFORCE_DX(dev->CreateTexture(kNormalMapSz,kNormalMapSz,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&smNoiseLUT,0))
              ("Failed to create water noise LUT texture.");
    nFillNoiseLUT();
              
    // Compile the shader.
    nCompileShader("data/shaders/water.fx",smShader);
    nGetGeoWaveParams();
    nGetTexWaveParams();
    
    // State block.
    dev->BeginStateBlock();
    dev->SetRenderState(D3DRS_ZENABLE,TRUE);
    dev->SetRenderState(D3DRS_LIGHTING,FALSE);
    dev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
    dev->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
    dev->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
    dev->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);
    dev->EndStateBlock(&smRenderStateBlock);

    smValidSharedData = true; 
  }
}

/*  ________________________________________________________________________ */
void Water::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  if(smValidSharedData)
  {
    SAFE_RELEASE(smRenderStateBlock);
    SAFE_RELEASE(smShader);
    SAFE_RELEASE(smNoiseLUT);
    SAFE_RELEASE(smCosLUT);
    SAFE_RELEASE(smRTTVB);
    SAFE_RELEASE(smCubemap);
    SAFE_RELEASE(smDeclRTT);
    SAFE_RELEASE(smDecl);    
    
    smValidSharedData = false;
  }
  SAFE_RELEASE(mNormalMap);
  SAFE_RELEASE(mVtxBuffer);
  SAFE_RELEASE(mIdxBuffer);
}

/*  ________________________________________________________________________ */
void Water::EnterRenderState(void)
/*! Enter batch render state.
*/
{
  ASSERT(!smInRenderState);
  smRenderStateBlock->Apply();
  smInRenderState = true;
}

/*  ________________________________________________________________________ */
void Water::LeaveRenderState(void)
/*! Leave batch render state.
*/
{
  ASSERT(smInRenderState);
  smInRenderState = false;
}

/*  ________________________________________________________________________ */
void Water::SetTranslation(float x,float y,float z)
/*! Set water plane translation.

    @param x  Translation along the X axis.
    @param y  Translation along the Y axis.
    @param z  Translation along the Z axis.
*/
{
  D3DXMatrixTranslation(&mMatTranslate,x,y,z);
} 

/*  ________________________________________________________________________ */
void Water::SetRotation(float x,float y,float z,float w)
/*! Set water plane rotation.

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
void Water::SetScale(float x,float y,float z)
/*! Set water plane scale.

    @param x  Scale along the X axis.
    @param y  Scale along the Y axis.
    @param z  Scale along the Z axis.
*/
{
  D3DXMatrixScaling(&mMatScale,x,y,z);
} 

/*  ________________________________________________________________________ */
void Water::Update(void)
/*! 
*/
{
  // Only update if enough time has passed.
  mWaveSimClock.Update();
  if(mWaveSimClock.Accumulated() >= 0.07)
  {
    mWaveSimTime += kWaterTimeStep;
    mWaveSimClock.Reset();
    
    // Update wave information.
    //nUpdateGeoWaves(kWaterTimeStep);
    //nUpdateTexWaves(kWaterTimeStep);
   // nUpdateBallWaves(kWaterTimeStep);
    //
    //// Regenerate normal map.
   // nRenderNormalMap();
  }
}

/*  ________________________________________________________________________ */
void Water::Disturb(float x,float y)
/*! 
*/
{
BallWaveInfo  w;

  // Set time and power.
  w.force = 1.0f;
  w.time  = 0.0f;

  // Store origin.
  w.x = x;
  w.y = y;  
  
  // Store wave.
  mBallWaves.push_back(w);
}

/*  ________________________________________________________________________ */
void Water::Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &/*eyeVec*/)
/*! Render the water plane.

    @param eye  The location of the camera, in world space.
*/
{
LPDIRECT3DDEVICE9 dev = mRenderer->GetDevice();

  // Set up matrices.
D3DXMATRIX  world;
D3DXMATRIX  view;
D3DXMATRIX  proj;

  world = mMatRotate * mMatTranslate;
  dev->SetTransform(D3DTS_WORLD,&world);
  dev->GetTransform(D3DTS_VIEW,&view);
  dev->GetTransform(D3DTS_PROJECTION,&proj);
  
D3DXMATRIX   vp  = view * proj;
D3DXMATRIX   wn;
D3DXVECTOR4  cam(eyePos.x,eyePos.y,eyePos.z,1.0f);

  // See if we should throw this plane away because it's
  // backfacing. The point (0,0,0) is on every plane (in
  // model space).
D3DXVECTOR4  planePt(0.0f,0.0f,0.0f,1.0f);
D3DXVECTOR3  planeVec;

  D3DXVec4Transform(&planePt,&planePt,&world);
  planeVec = D3DXVECTOR3(planePt.x,planePt.y,planePt.z);
  planeVec = eyePos - planeVec;
  
  // Test the dot product to determine facing.
  isRendered = false;
  if(D3DXVec3Dot(&planeVec,&mWorldNormal) < 0.0f)
    return;
  isRendered = true;

  // Enter render mode.
  if(!smInRenderState)
    smRenderStateBlock->Apply();
  
  dev->SetVertexDeclaration(smDecl);
  dev->SetIndices(mIdxBuffer);
  dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));

  // Compute normal transform matrix.
  D3DXMatrixTranspose(&wn,&world);
  D3DXMatrixInverse(&wn,0,&wn);
  
D3DXVECTOR3  sunVec = Skybox::GetSunVector();
D3DXVECTOR4  sun(sunVec.x,sunVec.y,sunVec.z,1.0f);

  smShader->SetTechnique("WaterV11P11");
  smShader->SetMatrix("gMatW",&world);
  smShader->SetMatrix("gMatWN",&wn);
  smShader->SetMatrix("gMatVP",&vp);
  smShader->SetVector("gEye",&cam);
  smShader->SetVector("gLight",&sun);
  nSetGeoWaveParams();
  
  // Render the plane.
UINT  totalPasses = 0;

  smShader->Begin(&totalPasses,0);
  for(UINT pass = 0; pass < totalPasses; pass++)
  {
    // Render each strip.
    D3D_SHADERPASS_BEGIN(smShader,pass);
    for(unsigned int i = 0; i < mNumStrips; ++i)
      dev->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP,0,0,mNumVerts,2 * (mRX + 1) * i,2 * mRX);
    D3D_SHADERPASS_END(smShader);
  }
  smShader->End();
}

/*  ________________________________________________________________________ */
void Water::nFillBuffers(void)
/*! Generate the vertices of the water plane.
*/
{
Vertex *vtx     = 0;
short  *idx     = 0;
float   x       = 0.0f;
float   y       = 0.0f;
float   u       = 0.0f;
float   v       = 0.0f;
float   step_x  = mSX / static_cast< float >(mRX);
float   step_y  = mSY / static_cast< float >(mRY);
float   step_u  = 1.0f / static_cast< float >(mRX);
float   step_v  = 1.0f / static_cast< float >(mRY);
float   bias_x  = -(mSX / 2.0f);  // Bias to center in the X at (0,0).
float   bias_y  = -(mSY / 2.0f);  // Bias to center in the Y at (0,0).

  // Lock buffers.
  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock vertex buffer.");
  ENFORCE_DX(mIdxBuffer->Lock(0,0,reinterpret_cast< void** >(&idx),0))
            ("Failed to lock index buffer.");
  
  // Generate a grid of vertices, centered at the origin,
  // in the XY plane.
  for(unsigned int i = 0; i < mRY + 1; ++i)
  {
    for(unsigned int j = 0; j < mRX + 1; ++j)
    {
      vtx->pos    = D3DXVECTOR3(x + bias_x,y + bias_y,0.0f);
      vtx->normal = D3DXVECTOR3(0.0f,0.0f,-1.0f);
      vtx->texc   = D3DXVECTOR2(u,v);
      ++vtx;
      
      x += step_x;
      u += step_u;
    }
    x = u = 0.0f;
    y += step_y;
    v += step_v;
  }
  
  // Generate a series of indices that will render each row of the
  // grid as a single triangle strip.
  for(unsigned int i = 0; i < mNumStrips; ++i)
  {
  int  c = 0;

    // Odd vertices are taken from the row we're on,
    // even from the row below. This gives us a staggering
    // pattern that we need to make a triangle strip.
    for(unsigned int j = 0; j < (2 * (mRX + 1)); ++j)
    {
      if(j % 2 == 0)
        *idx++ = static_cast< short >(i * (mRX + 1) + c);
      else
        *idx++ = static_cast< short >((i + 1) * (mRX + 1) + c),c++;
    }
  }

  // Unlock buffers.
  ENFORCE_DX(mIdxBuffer->Unlock())
            ("Failed to unlock index buffer.");
  ENFORCE_DX(mVtxBuffer->Unlock())
            ("Failed to unlock vertex buffer.");
}

/*  ________________________________________________________________________ */
void Water::nFillBuffersRTT(void)
/*! Generate the vertices of the RTT quad.
*/
{
VertexRTT *vtx = 0;

  // Lock buffer.
  ENFORCE_DX(smRTTVB->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock vertex buffer.");
       
  vtx[0].pos  = D3DXVECTOR3(-1.0f,1.0f,1.0f);
  vtx[0].texc = D3DXVECTOR2(0.5f / kNormalMapSz,
                            0.5f / kNormalMapSz + 1.0f);
  vtx[1].pos  = D3DXVECTOR3(1.0f,1.0f,1.0f);
  vtx[1].texc = D3DXVECTOR2(0.5f / kNormalMapSz + 1.0f,
                            0.5f / kNormalMapSz + 1.0f);
  vtx[2].pos  = D3DXVECTOR3(-1.0f,-1.0f,1.0f);
  vtx[2].texc = D3DXVECTOR2(0.5f / kNormalMapSz,
                            0.5f / kNormalMapSz);
  vtx[3].pos  = D3DXVECTOR3(1.0f,-1.0f,1.0f);
  vtx[3].texc = D3DXVECTOR2(0.5f / kNormalMapSz + 1.0f,
                            0.5f / kNormalMapSz);
                            
  ENFORCE_DX(smRTTVB->Unlock())
            ("Failed to unlock vertex buffer.");
}

/*  ________________________________________________________________________ */
void Water::nFillCosineLUT(void)
/*! Fill cosine lookup table texture.
*/
{
D3DLOCKED_RECT  lr;
  
  ENFORCE_DX(smCosLUT->LockRect(0,&lr,0,0))
            ("Failed to lock cosine LUT texture.");

unsigned long *data = static_cast< unsigned long* >(lr.pBits);

  // For each U texture coordinate, calculate cos(u2pi) and
  // write it into the texture. U is scaled by 2pi in order
  // to confine the entire period of the cosine curve to [0..1].
  for(int i = 0; i < kNormalMapSz; ++i)
  {
  float  u = static_cast< float >(i) / static_cast< float >(kNormalMapSz - 1) * 2.0f * D3DX_PI;
  float  c = static_cast< float >(::cos(u));
  float  s = static_cast< float >(::sin(u));
  
    s *= 0.5f;
    s += 0.5f;
    s = static_cast< float >(pow(s,1.0f));
    c *= s;
    
    // Convert to a color.
  unsigned char  cc = static_cast< unsigned char >((c * 0.5f + 0.5f) * 255.999f);
    
    // Pack the cosine into the red and green channels of the texture.
    data[i] = D3DCOLOR_RGBA(cc,cc,0xFF,0xFF);
  }

  ENFORCE_DX(smCosLUT->UnlockRect(0))
            ("Failed to unlock cosine LUT texture.");
}

/*  ________________________________________________________________________ */
void Water::nFillNoiseLUT(void)
/*! Fill bias noise lookup texture.
*/
{
D3DLOCKED_RECT  lr;
  
  ENFORCE_DX(smNoiseLUT->LockRect(0,&lr,0,0))
            ("Failed to lock noise LUT texture.");

unsigned long *data = static_cast< unsigned long* >(lr.pBits);

  for(int i = 0; i < kNormalMapSz; ++i)
  {
    for(int j = 0; j < kNormalMapSz; ++j)
    {
    float          x = gRNG.rand_float(0.0f,1.0f);
    float          y = gRNG.rand_float(0.0f,1.0f);
    unsigned char  r = static_cast< unsigned char>(x * 255.999f);
    unsigned char  g = static_cast< unsigned char>(y * 255.999f);
  
      // Pack the noise into the red and green channels.
      data[j] = D3DCOLOR_RGBA(r,g,0xFF,0xFF);
    }

    // Next line.
    data += lr.Pitch / 4;
  }

  ENFORCE_DX(smNoiseLUT->UnlockRect(0))
            ("Failed to unlock noise LUT texture.");
}

/*  ________________________________________________________________________ */
void Water::nInitGeoWaves(void)
/*! Initialize geometry base waves.
*/
{
  nInitGeoState();
  for(int i = 0; i < kWaterNumGeoWaves; ++i)
    nInitGeoWave(i);
}

/*  ________________________________________________________________________ */
void Water::nInitGeoState(void)
/* Initialize geometry-based wave simulation globals.
*/
{
  mGeoState.chop = 2.5f;
  mGeoState.angleDeviation = 15.f;
  mGeoState.windDir.x = 0;
  mGeoState.windDir.y = 1.f;

  mGeoState.minLength = 15.f;
  mGeoState.maxLength = 25.f;
  mGeoState.ampOverLen = 0.1f;

  mGeoState.envHeight = -50.f;
  mGeoState.envRadius = 100.f;

  mGeoState.transIdx = 0;
  mGeoState.transDel = -1.f / 6.f;

  mGeoState.specAtten = 1.f;
  mGeoState.specEnd = 200.f;
  mGeoState.specTrans = 100.f;
}

/*  ________________________________________________________________________ */
void Water::nInitGeoWave(int i)
/*! Initialize an individual geometry wave.
*/
{
  // Generate values.
  mGeoWaves[i].amp = gRNG.rand_float(0.0f,kGeoWaveAmpLimit);
  mGeoWaves[i].frq = gRNG.rand_float(0.0f,kGeoWaveFrqLimit);
  mGeoWaves[i].spd = gRNG.rand_float(0.0f,kGeoWaveSpdLimit);
  
  // Generate wave direction. This should be normalized.
D3DXVECTOR2  dir(gRNG.rand_float(-1.0f,1.0f),
                 gRNG.rand_float(-1.0f,1.0f));

  D3DXVec2Normalize(&dir,&dir); 
  mGeoWaves[i].direction.x = dir.x;
  mGeoWaves[i].direction.y = dir.y;
  
  // Set fade.
  mGeoWaves[i].fade = 1.0f;
}

/*  ________________________________________________________________________ */
void Water::nUpdateGeoWaves(float t)
/*! Update geometry waves.

    @param t  Update timestep.
*/
{
  for(int i = 0; i < kWaterNumGeoWaves; ++i)
    nUpdateGeoWave(t,i);
}

/*  ________________________________________________________________________ */
void Water::nUpdateGeoWave(float /*t*/,int /*i*/)
/*! Update an individual geometry wave.

    @param t  Update timestep.
    @param i  Wave index to update.
*/
{
}

/*  ________________________________________________________________________ */
void Water::nInitTexWaves(void)
/*! Initialize texture-based wave simulation.
*/
{
  nInitTexState();
  for(int i = 0; i < kWaterNumTexWaves; ++i)
    nInitTexWave(i);
}

/*  ________________________________________________________________________ */
void Water::nInitTexState(void)
/* Initialize texture-based wave simulation globals.
*/
{
  mTexState.noise          =  0.2f;
  mTexState.chop           =  1.0f;
  mTexState.angleDeviation = 15.f;
  mTexState.windDir.x      =  0.0f;
  mTexState.windDir.y      =  1.0f;
  mTexState.maxLength      = 10.f;
  mTexState.minLength      =  1.0f;
  mTexState.ampOverLen     =  0.1f;
  mTexState.rippleScale    = 25.0f;
  mTexState.spdDeviation   =  0.1f;

  mTexState.transIdx = 0;
  mTexState.transDel = -1.0f / 5.0f;
}

/*  ________________________________________________________________________ */
void Water::nInitTexWave(int i)
/*! Initialize an individual texture wave.
*/
{
float  rads = gRNG.rand_float(-1.0f,1.0f) * mTexState.angleDeviation * D3DX_PI / 180.f;
float  dx   = static_cast< float >(::sin(rads));
float  dy   = static_cast< float >(::cos(rads));
float  tx   = dx;
  
  // Set rotation.
  dx = mTexState.windDir.y * dx - mTexState.windDir.x * dy;
  dy = mTexState.windDir.x * tx + mTexState.windDir.y * dy;
  
  // Compute length.
float  maxLen = mTexState.maxLength * kNormalMapSz / mTexState.rippleScale;
float  minLen = mTexState.minLength * kNormalMapSz / mTexState.rippleScale;
float  len    = static_cast< float >(i) / static_cast< float >(kWaterNumTexWaves - 1) * (maxLen - minLen) + minLen;
float  reps   = static_cast< float >(kNormalMapSz) / len;

  // Finalize rotation based on how wave will repeat.
  // Only certain values will allow the texture to tile.
  dx *= reps;
  dy *= reps;
  dx = static_cast< float >(static_cast< int >(dx >= 0 ? dx + 0.5f : dx - 0.5f));
  dy = static_cast< float >(static_cast< int >(dy >= 0 ? dy + 0.5f : dy - 0.5f));
  mTexWaves[i].rotateScale.x = dx;
  mTexWaves[i].rotateScale.y = dy;

float  k = static_cast< float >(1.0f / ::sqrt(dx * dx + dy * dy));
  
  // Length, frequency, phase.
  mTexWaves[i].len   = static_cast< float >(kNormalMapSz) * k;
  mTexWaves[i].frq   = D3DX_PI * 2.0f / mTexWaves[i].len;
  mTexWaves[i].phase = gRNG.rand_float(0.0f,1.0f);
  
  // Amplitude must obey a ratio.
  mTexWaves[i].amp = mTexWaves[i].len * mTexState.ampOverLen;
  
  // Wave direction.
  mTexWaves[i].direction.x = dx * k;
  mTexWaves[i].direction.y = dy * k;

  // Fade starts at 1.
  mTexWaves[i].fade = 1.0f;

  // Compute speed.
float speed = static_cast< float >(1.0f / ::sqrt(mTexWaves[i].len / (2.0f * D3DX_PI * kWaveGravity)) ) / 3.0f;
  
  speed *= 1.0f + gRNG.rand_float(-1.0f,1.0f) * mTexState.spdDeviation;
  mTexWaves[i].spd = speed;
}

/*  ________________________________________________________________________ */
void Water::nUpdateTexWaves(float t)
/* Update texture-based wave simulation.
   
   @param t  Update timestep.
*/
{
  for(int i = 0; i < kWaterNumTexWaves; ++i)
    nUpdateTexWave(t,i);
}

/*  ________________________________________________________________________ */
void Water::nUpdateTexWave(float t,int i)
/*! Update an individual texture-based wave.

    @param t  Update timestep.
    @param i  Wave index to update.
*/
{
  // If this is the fading wave...
  if(i == mTexState.transIdx)
  {
    mTexWaves[i].fade += mTexState.transDel * t;
    if(mTexWaves[i].fade < 0.0f)
    {
      // Wave died. Recreate and fade it up.
      nInitTexWave(i);
      
      // Make sure to flip the fade info or we'll have a jumpy
      // simulation, which is freakin' ugly.
      mTexWaves[i].fade  = 0.0f;
      mTexState.transDel = -mTexState.transDel;
    }
    else if(mTexWaves[i].fade > 1.0f)
    {
      // Wave's all faded up. Fade the next one down.
      mTexWaves[i].fade  = 1.0f;
      mTexState.transDel = -mTexState.transDel;
      if(++mTexState.transIdx >= kWaterNumTexWaves)
        mTexState.transIdx = 0;
    }
  }
  mTexWaves[i].phase -= t * mTexWaves[i].spd;
  mTexWaves[i].phase -= int(mTexWaves[i].phase);
}

/*  ________________________________________________________________________ */
void Water::nUpdateBallWaves(float t)
/*! Update ball-based waves.

    @param t  Update timestep.
*/
{
Vertex *vtx = 0;

  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
            ("Failed to lock water vertex buffer during wave update.");

  for(unsigned int i = 0; i < mBallWaves.size(); ++i)
    nUpdateBallWave(t,i,vtx);
  
  ENFORCE_DX(mVtxBuffer->Unlock())
            ("Failed to unlock water vertex buffer during wave update.");
}

/*  ________________________________________________________________________ */
void Water::nUpdateBallWave(float t,int i,Water::Vertex *vtx)
/*! Update an individual ball wave.

    @param t  Update timestep.
    @param i  Wave index to update.
*/
{
  // Update time (dist from origin).
  mBallWaves[i].time += 10.0f * t;

  // Loop through vertices. Displace the vertex if its inside the wave front.
  // The front is a ring, the inner dimension is of radius (time - force) and
  // the outer dimension (time + force).
  for(unsigned int c = 0; c < mRY; ++c)
  {
    for(unsigned int r = 0; r < mRX; ++r)
    {
    D3DXVECTOR2  vec(mBallWaves[i].x - vtx->pos.x,mBallWaves[i].y - vtx->pos.y);
    float        len = D3DXVec2Length(&vec);
    
      // If the length of the vector is between the inner and outer radius,
      // this vertex is changed by the wave.
      if(len > (mBallWaves[i].time - mBallWaves[i].force) && len < (mBallWaves[i].time + mBallWaves[i].force))
      {
        vtx->pos.z    = scast<float>(1.0f * 1.0f / (10.0 * mBallWaves[i].time));
        vtx->normal.x = -vec.x;
        vtx->normal.y = -vec.y;
        vtx->normal.y = 1.0f;
        D3DXVec3Normalize(&vtx->normal,&vtx->normal);
      }
      else
      {
        vtx->pos.z    = 0.0f;
       // vtx->normal.x = 0.0f;
     //   vtx->normal.y = 0.0f;
      }
      ++vtx;
    }
  }  
}

/*  ________________________________________________________________________ */
void Water::nGetTexWaveParams(void)
/*! Get texture wave shader parameters.
*/
{
  // Grab wave translation handles.
  smTexWaveParams.trans[0]  = smShader->GetParameterByName(0,"gTrans0");
  smTexWaveParams.trans[1]  = smShader->GetParameterByName(0,"gTrans1");
  smTexWaveParams.trans[2]  = smShader->GetParameterByName(0,"gTrans2");
  smTexWaveParams.trans[3]  = smShader->GetParameterByName(0,"gTrans3");
  smTexWaveParams.trans[4]  = smShader->GetParameterByName(0,"gTrans4");
  smTexWaveParams.trans[5]  = smShader->GetParameterByName(0,"gTrans5");
  smTexWaveParams.trans[6]  = smShader->GetParameterByName(0,"gTrans6");
  smTexWaveParams.trans[7]  = smShader->GetParameterByName(0,"gTrans7");
  smTexWaveParams.trans[8]  = smShader->GetParameterByName(0,"gTrans8");
  smTexWaveParams.trans[9]  = smShader->GetParameterByName(0,"gTrans9");
  smTexWaveParams.trans[10] = smShader->GetParameterByName(0,"gTrans10");
  smTexWaveParams.trans[11] = smShader->GetParameterByName(0,"gTrans11");
  smTexWaveParams.trans[12] = smShader->GetParameterByName(0,"gTrans12");
  smTexWaveParams.trans[13] = smShader->GetParameterByName(0,"gTrans13");
  smTexWaveParams.trans[14] = smShader->GetParameterByName(0,"gTrans14");
  smTexWaveParams.trans[15] = smShader->GetParameterByName(0,"gTrans15");

  // Grab wave coefficient handles.
  smTexWaveParams.coeff[0]  = smShader->GetParameterByName(0,"gCoeff0");
  smTexWaveParams.coeff[1]  = smShader->GetParameterByName(0,"gCoeff1");
  smTexWaveParams.coeff[2]  = smShader->GetParameterByName(0,"gCoeff2");
  smTexWaveParams.coeff[3]  = smShader->GetParameterByName(0,"gCoeff3");
  smTexWaveParams.coeff[4]  = smShader->GetParameterByName(0,"gCoeff4");
  smTexWaveParams.coeff[5]  = smShader->GetParameterByName(0,"gCoeff5");
  smTexWaveParams.coeff[6]  = smShader->GetParameterByName(0,"gCoeff6");
  smTexWaveParams.coeff[7]  = smShader->GetParameterByName(0,"gCoeff7");
  smTexWaveParams.coeff[8]  = smShader->GetParameterByName(0,"gCoeff8");
  smTexWaveParams.coeff[9]  = smShader->GetParameterByName(0,"gCoeff9");
  smTexWaveParams.coeff[10] = smShader->GetParameterByName(0,"gCoeff10");
  smTexWaveParams.coeff[11] = smShader->GetParameterByName(0,"gCoeff11");
  smTexWaveParams.coeff[12] = smShader->GetParameterByName(0,"gCoeff12");
  smTexWaveParams.coeff[13] = smShader->GetParameterByName(0,"gCoeff13");
  smTexWaveParams.coeff[14] = smShader->GetParameterByName(0,"gCoeff14");
  smTexWaveParams.coeff[15] = smShader->GetParameterByName(0,"gCoeff15");

  // Scaling handle.
  smTexWaveParams.rescale = smShader->GetParameterByName(0,"gRescale");

  // Noise handles.
  smTexWaveParams.noiseBias[0] = smShader->GetParameterByName(0,"gNoiseBias0_00");
  smTexWaveParams.noiseBias[1] = smShader->GetParameterByName(0,"gNoiseBias0_10");
  smTexWaveParams.noiseBias[2] = smShader->GetParameterByName(0,"gNoiseBias1_00");
  smTexWaveParams.noiseBias[3] = smShader->GetParameterByName(0,"gNoiseBias1_10");

  // Set noise vectors.
D3DXVECTOR4  init(20.0f,0.0f,0.0f,0.0f);

  smShader->SetVector(smTexWaveParams.noiseBias[0],&init);
  smShader->SetVector(smTexWaveParams.noiseBias[2],&init);
  init.x = 0.0f;
  init.y = 20.f;
  smShader->SetVector(smTexWaveParams.noiseBias[1],&init);
  smShader->SetVector(smTexWaveParams.noiseBias[3],&init);

  // Get scale bias.
  smTexWaveParams.scaleBias = smShader->GetParameterByName(0,"gScaleBias");
}

/*  ________________________________________________________________________ */
void Water::nSetTexWaveParams(void)
/*! Set texture wave shader parameters.
*/
{
  // Set translations and coeffs.
  for(int i = 0; i < kWaterNumTexWaves; ++i)
  {
  float        normalScale = mTexWaves[i].fade / static_cast< float >(kWaterBumpPassCnt);
  D3DXVECTOR4  utrans(mTexWaves[i].rotateScale.x,
                      mTexWaves[i].rotateScale.y,
                      0.0f,
                      mTexWaves[i].phase);
  D3DXVECTOR4  coef(mTexWaves[i].direction.x * normalScale,
                    mTexWaves[i].direction.y * normalScale,
                    1.0f,
                    1.0f);      
                           
    smShader->SetVector(smTexWaveParams.trans[i],&utrans);
    smShader->SetVector(smTexWaveParams.coeff[i],&coef);
  }

  // Set noise bias.
D3DXVECTOR4  xform;
  
  smShader->GetVector(smTexWaveParams.noiseBias[0],&xform);
  xform.w += kWaterTimeStep * 0.1f;
  smShader->SetVector(smTexWaveParams.noiseBias[0],&xform);
  smShader->GetVector(smTexWaveParams.noiseBias[3],&xform);
  xform.w += kWaterTimeStep * 0.1f;
  smShader->SetVector(smTexWaveParams.noiseBias[3],&xform);

float        s  = 0.5f / (static_cast< float >(kWaterBumpPerPass) + mTexState.noise);
float        sb = 0.5f * mTexState.noise / (static_cast< float >(kWaterBumpPassCnt) + mTexState.noise);
D3DXVECTOR4  rescale(s,s,1.0f,1.0f);
D3DXVECTOR4  scalebias(sb,sb,0.0f,1.0f);
  
  // Set bias vectors.
  smShader->SetVector(smTexWaveParams.rescale,&rescale);
  smShader->SetVector(smTexWaveParams.scaleBias,&scalebias);
  
  // Set textures.
  smShader->SetTexture("gCosLUT",smCosLUT);
  smShader->SetTexture("gNoiseLUT",smNoiseLUT);
}

/*  ________________________________________________________________________ */
void Water::nGetGeoWaveParams(void)
/*! Get geometry wave shader parameters.
*/
{
  // Get time.
	smGeoWaveParams.time = smShader->GetParameterByName(0,"gTime");
  
  // Get wave properties.
  smGeoWaveParams.amps = smShader->GetParameterByName(0,"gWAmp");
  smGeoWaveParams.frqs = smShader->GetParameterByName(0,"gWFrq");
  smGeoWaveParams.spds = smShader->GetParameterByName(0,"gWSpd");
  smGeoWaveParams.dirx = smShader->GetParameterByName(0,"gWVX");
  smGeoWaveParams.diry = smShader->GetParameterByName(0,"gWVY");
  
  // Get textures.
  smGeoWaveParams.envMap  = smShader->GetParameterByName(0,"gEnvMap");
  smGeoWaveParams.bumpMap = smShader->GetParameterByName(0,"gBumpMap");
}

/*  ________________________________________________________________________ */
void Water::nSetGeoWaveParams(void)
/*! Set geometry wave shader parameters.
*/
{
  // Simulation time.
  smShader->SetFloat(smGeoWaveParams.time,mWaveSimTime);

  // Wave properties.
D3DXVECTOR4  params;

  params = D3DXVECTOR4(mGeoWaves[0].amp,
                       mGeoWaves[1].amp,
                       mGeoWaves[2].amp,
                       mGeoWaves[3].amp);
  smShader->SetVector(smGeoWaveParams.amps,&params);
  params = D3DXVECTOR4(mGeoWaves[0].frq,
                       mGeoWaves[1].frq,
                       mGeoWaves[2].frq,
                       mGeoWaves[3].frq);
  smShader->SetVector(smGeoWaveParams.frqs,&params);
  params = D3DXVECTOR4(mGeoWaves[0].spd,
                       mGeoWaves[1].spd,
                       mGeoWaves[2].spd,
                       mGeoWaves[3].spd);
  smShader->SetVector(smGeoWaveParams.spds,&params);
  params = D3DXVECTOR4(mGeoWaves[0].direction.x,
                       mGeoWaves[1].direction.x,
                       mGeoWaves[2].direction.x,
                       mGeoWaves[3].direction.x);
  smShader->SetVector(smGeoWaveParams.dirx,&params);
  params = D3DXVECTOR4(mGeoWaves[0].direction.y,
                       mGeoWaves[1].direction.y,
                       mGeoWaves[2].direction.y,
                       mGeoWaves[3].direction.y);
  smShader->SetVector(smGeoWaveParams.diry,&params);

  // Textures.
  smShader->SetTexture(smGeoWaveParams.envMap,smCubemap);
  smShader->SetTexture(smGeoWaveParams.bumpMap,mNormalMap);
}


/*  ________________________________________________________________________ */
void Water::nRenderNormalMap(void)
/*! Render the water's normal map.
*/
{
LPDIRECT3DDEVICE9   dev        = mRenderer->GetDevice();
LPDIRECT3DSURFACE9  target     = 0;
LPDIRECT3DSURFACE9  backbuffer = 0;
float               sz         = static_cast< float >(kNormalMapSz);

  // Get the current render surface.
  ENFORCE_DX(dev->GetRenderTarget(0,&backbuffer))
            ("Failed to save backbuffer during water normal render.");
  
  // Get the surface we're going to render into.
  ENFORCE_DX(mNormalMap->GetSurfaceLevel(0,&target))
            ("Failed to acquire water normal surface.");
  
  // Set the render target and begin render.
  ENFORCE_DX(dev->SetRenderTarget(0,target))
            ("Failed to set water normal surface as rendertarget.");
  mRenderer->Clear();
  mRenderer->BeginRender();
  
  // Setup.
D3DXMATRIX  mat;

  smShader->SetTechnique("WaterNormalV11P11");
  nSetTexWaveParams();

  // Create a matrix to transform the rectangle to the desired
  // position and size.
  D3DXMatrixScaling(&mat,sz,sz,0.0f);
  smShader->SetMatrix("gMatW",&mat);
  
  // Create a projection matrix.
  D3DXMatrixOrthoLH(&mat,sz,sz,0.0f,2000.0f);
  smShader->SetMatrix("gMatVP",&mat);
  
  // Set stream.
  dev->SetVertexDeclaration(smDeclRTT);
  dev->SetStreamSource(0,smRTTVB,0,sizeof(VertexRTT));

  // Render the texture.
UINT  totalPasses = 0;

  smShader->Begin(&totalPasses,0);
  for(UINT pass = 0; pass < totalPasses; ++pass)
  {
    D3D_SHADERPASS_BEGIN(smShader,pass);
    dev->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
    D3D_SHADERPASS_END(smShader);
  }
  smShader->End();
  
  // Finish render and restore old render target.
  mRenderer->EndRender();
  ENFORCE_DX(dev->SetRenderTarget(0,backbuffer))
            ("Failed to set restore backbuffer during water normal render.");
}
