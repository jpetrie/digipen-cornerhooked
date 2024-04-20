/*! ========================================================================

      @file    Skybox.h
      @author  jmp
      @brief   Implementation of skybox object.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "Skybox.h"

#include "nsl_tools_string.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  // size
  const float  kSkyboxSz = 700.0f;  // Distance from (0,0,0) of all planes.
  
  // counts
  const int  kMaxVertices  = 36;
  const int  kNumTriangles = 12;
  
  // plane indices
  const int  kFrontIdx  = 0;
  const int  kBackIdx   = 1;
  const int  kTopIdx    = 2;
  const int  kBottomIdx = 3;
  const int  kLeftIdx   = 4;
  const int  kRightIdx  = 5;
  
  // default ini values
  const std::string  kDefUIBaseColor        = "100,50,50,255";
  const std::string  kDefUITextColor        = "255,255,255,255";
  const std::string  kDefUIDisableTextColor = "255,127,127,127";
  const std::string  kDefUILabelTextColor   = "255,200,200,255";
  const std::string  kDefUIPanelColor       = "50,50,50,255";
  const std::string  kDefSunVector          = "1,1,1";
  const std::string  kDefSunColor           = "255,1,1,1";
}


/*                                                                 variables
---------------------------------------------------------------------------- */

int          Skybox::smSkyState            = 0;
D3DXVECTOR3  Skybox::smSunVector           = D3DXVECTOR3(0.0f,0.0f,0.0f);
DWORD        Skybox::smSunColor            = 0;
DWORD        Skybox::smUIBaseColor         = 0;
DWORD        Skybox::smUITextColor         = 0;
DWORD        Skybox::smUIDisableTextColor  = 0;
DWORD        Skybox::smUILabelTextColor    = 0;
DWORD        Skybox::smUIPanelColor        = 0;

namespace
{
  // shader stream declaration
  D3DVERTEXELEMENT9 gDeclShell[] =
  {
    { 0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    { 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
  };
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Skybox::Skybox(Graphics::Renderer *renderer,int hours)
/*! Constructor.

    @param renderer  Renderer to attach to.
    @param hours     Hours (0-23) of the current time.
*/
: Graphics::Primitive(renderer),
  mVtxBuffer(0),
  mShader(0),mDecl(0),
  mRenderStateBlock(0)
{
  // Pick sky state based on time of day.
  smSkyState = hours / (24 / kSkyStateCnt);
  
  // Read UI and other state properties from the INI file.
std::string  iniFile = nsl::stformat("data/skybox/%s/skybox.ini",kSkyStates[smSkyState].c_str());
  
  smUIBaseColor        = nReadColorVector("UI","Base",kDefUIBaseColor,iniFile);
  smUITextColor        = nReadColorVector("UI","Text",kDefUITextColor,iniFile);
  smUIDisableTextColor = nReadColorVector("UI","DisableText",kDefUIDisableTextColor,iniFile);
  smUILabelTextColor   = nReadColorVector("UI","LabelText",kDefUILabelTextColor,iniFile);
  smUIPanelColor       = nReadColorVector("UI","Panel",kDefUIPanelColor,iniFile);

  smSunVector = nReadLightVector("Environs","SunVector",kDefSunVector,iniFile);
  smSunColor  = nReadColorVector("Environs","SunColor",kDefSunColor,iniFile);

  // Set up texture data.
  for(unsigned int i = 0; i < 6; ++i)
    mTextures[i] = 0;
  mTextureNames[kFrontIdx]  = nsl::stformat("data/skybox/%s/%s",kSkyStates[smSkyState].c_str(),kSkyFront.c_str());
  mTextureNames[kBackIdx]   = nsl::stformat("data/skybox/%s/%s",kSkyStates[smSkyState].c_str(),kSkyBack.c_str());
  mTextureNames[kLeftIdx]   = nsl::stformat("data/skybox/%s/%s",kSkyStates[smSkyState].c_str(),kSkyLeft.c_str());
  mTextureNames[kRightIdx]  = nsl::stformat("data/skybox/%s/%s",kSkyStates[smSkyState].c_str(),kSkyRight.c_str());
  mTextureNames[kTopIdx]    = nsl::stformat("data/skybox/%s/%s",kSkyStates[smSkyState].c_str(),kSkyTop.c_str());
  mTextureNames[kBottomIdx] = nsl::stformat("data/skybox/%s/%s",kSkyStates[smSkyState].c_str(),kSkyBottom.c_str());
  
  // Face normals (world space).
  mNormals[kFrontIdx]  = D3DXVECTOR3(0.0f,0.0f,-1.0f);
  mNormals[kBackIdx]   = D3DXVECTOR3(0.0f,0.0f,1.0f);
  mNormals[kLeftIdx]   = D3DXVECTOR3(1.0f,0.0f,0.0f);
  mNormals[kRightIdx]  = D3DXVECTOR3(-1.0f,0.0f,0.0f);
  mNormals[kTopIdx]    = D3DXVECTOR3(0.0f,-1.0f,0.0f);
  mNormals[kBottomIdx] = D3DXVECTOR3(0.0f,1.0f,0.0f);
  
  RestoreDeviceObjects();
}

/*  ________________________________________________________________________ */
Skybox::~Skybox(void)
/*! Destructor.
*/
{
  ReleaseDeviceObjects();
}


/*  ________________________________________________________________________ */
void Skybox::RestoreDeviceObjects(void)
/*! Restore D3D device objects.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  // Create the textures.
  for(unsigned int i = 0; i < 6; ++i)
    ENFORCE_DX(D3DXCreateTextureFromFileEx(dev,mTextureNames[i].c_str(),
                                           0,0,
                                           1,
                                           0,
                                           D3DFMT_UNKNOWN,
                                           D3DPOOL_DEFAULT,
                                           D3DX_DEFAULT,
                                           D3DX_DEFAULT,
                                           0,0,0,
                                           &mTextures[i]))
              ("Failed to create skybox texture ")(i)(": ")(mTextureNames[i].c_str());
            
  // Create the vertex buffer.
  ENFORCE_DX(dev->CreateVertexBuffer(10 * 10 * sizeof(Vertex),D3DUSAGE_WRITEONLY,0,D3DPOOL_DEFAULT,&mVtxBuffer,0))
            ("Failed to create skybox vertex buffer.h");
	nFillVtxBuffer();

  // Stream declaration.
  ENFORCE_DX(dev->CreateVertexDeclaration(gDeclShell,&mDecl))
            ("Failed to create skybox stream declaration.");

  // Shader.
  nCompileShader("data/shaders/skybox.fx",mShader);

  // Create the render state block.
  dev->BeginStateBlock();
  dev->SetRenderState(D3DRS_LIGHTING,FALSE);
  dev->EndStateBlock(&mRenderStateBlock);
}

/*  ________________________________________________________________________ */
void Skybox::ReleaseDeviceObjects(void)
/*! Release D3D device objects.
*/
{
  SAFE_RELEASE(mRenderStateBlock);
  SAFE_RELEASE(mShader);
  SAFE_RELEASE(mDecl);
  SAFE_RELEASE(mVtxBuffer);
  for(unsigned int i = 0; i < 6; ++i)
    SAFE_RELEASE(mTextures[i]);
}

/*  ________________________________________________________________________ */
void Skybox::Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &eyeVec)
/*! Render the skybox.

    @param eye  The location of the eye in the world.
*/
{
LPDIRECT3DDEVICE9  dev = mRenderer->GetDevice();

  // Apply render state.
  mRenderStateBlock->Apply();
  dev->SetVertexDeclaration(mDecl);
  dev->SetStreamSource(0,mVtxBuffer,0,sizeof(Vertex));

  // Set world matrix. The skybox should not appear to move when the eye
  // position translates, so we center it on the eye at all times.
D3DXMATRIX  world;
D3DXMATRIX  view;
D3DXMATRIX  proj;

  D3DXMatrixTranslation(&world,eyePos.x,eyePos.y,eyePos.z);
  dev->SetTransform(D3DTS_WORLD,&world);
  dev->GetTransform(D3DTS_VIEW,&view);
  dev->GetTransform(D3DTS_PROJECTION,&proj);
  
  // Set shader inputs.
D3DXMATRIX  wvp = world * view * proj;

  mShader->SetMatrix("gMatWVP",&wvp);

  // Now render the box planes.
UINT  totPasses = 0;

  mShader->SetTechnique("SkyboxV11P11");
  mShader->Begin(&totPasses,0);
  for(UINT pass = 0; pass < totPasses; ++pass)
  {
    D3D_SHADERPASS_BEGIN(mShader,pass);
    for(unsigned int i = 0; i < 6; ++i)
    {
      if(D3DXVec3Dot(&mNormals[i],&eyeVec) < 0.0f)
      {
        mShader->SetTexture("gTexture",mTextures[i]);
        D3D_SHADERPASS_COMMIT(mShader);
        dev->DrawPrimitive(D3DPT_TRIANGLELIST,(6 * i),2);
      }
    }
    D3D_SHADERPASS_END(mShader);
  }
  mShader->End();
}


/*  ________________________________________________________________________ */
void Skybox::nFillVtxBuffer(void)
/*! Generate the vertices of the skybox.
*/
{
  const float F = 1.0f;	// full
  float Sx, Sy;			// shift
  D3DSURFACE_DESC ddsdesc;
  Vertex *vtx;

  ENFORCE_DX(mVtxBuffer->Lock(0,0,reinterpret_cast< void** >(&vtx),0))
          ("Failed to lock skybox vertex buffer.");

  mTextures[0]->GetLevelDesc( 0, &ddsdesc );
  Sx = 1.0f / (float)ddsdesc.Width;
  Sy = 1.0f / (float)ddsdesc.Height;

  // Front, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
	// Front, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  // Front, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
 
  // Front, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Front, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
  // Front, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  
	// Back, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  // Back, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  // Back, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  
  // Back, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  // Back, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Back, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
  
  // Front, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Front, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
	// Back, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  
  // Front, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
  // Back, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  // Back, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  
	// Back, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Front, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  // Front, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  
  // Back, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Back, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
  // Front, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;

  // Back, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  // Front, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  // Front, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  
  // Front, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Back, bottom, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
  // Back, top, left.
  vtx->pos = D3DXVECTOR3(-kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
  
  // Front, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;
  // Front, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,Sy);
  ++vtx;
    // Back, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  
  // Back, top, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,Sy);
  ++vtx;
  // Back, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,-kSkyboxSz);
  vtx->tex = D3DXVECTOR2(F-Sx,F-Sy);
  ++vtx;
  // Front, bottom, right.
  vtx->pos = D3DXVECTOR3(kSkyboxSz,-kSkyboxSz,kSkyboxSz);
  vtx->tex = D3DXVECTOR2(Sx,F-Sy);
  ++vtx;  
  
  ENFORCE_DX(mVtxBuffer->Unlock())
            ("Failed to unlock vertex buffer.");      
}

/*  ________________________________________________________________________ */
DWORD Skybox::nReadColorVector(const std::string &sec,const std::string &key,const std::string &def,const std::string &fn)
/*! Convert a color vector value from the skybox INI to a real color.

    @param  sec INI section.
    @param  key INI key.
    @param  def Default value.
    @param  fn  INI file name.
    
    @return
    A DWORD containing a D3D color value.
*/
{
char         buffer[256];
std::string  vecVal;

  ::GetPrivateProfileString(sec.c_str(),key.c_str(),def.c_str(),buffer,256,fn.c_str());
  vecVal = buffer;
  
  // Split it by commas and whitespace.
std::vector< std::string >  parts = nsl::stsplit_chr(vecVal,", ");

  // Omitted components are 0.
  while(parts.size() < 4)
    parts.push_back("0");
  
  // Transform into a color.
  return (D3DCOLOR_RGBA(lexical_cast< int >(parts[1]),
                        lexical_cast< int >(parts[2]),
                        lexical_cast< int >(parts[3]),
                        lexical_cast< int >(parts[0])));
}

/*  ________________________________________________________________________ */
D3DXVECTOR3 Skybox::nReadLightVector(const std::string &sec,const std::string &key,const std::string &def,const std::string &fn)
/*! Convert a light vector value from the skybox INI to a real vector.

    @param  sec INI section.
    @param  key INI key.
    @param  def Default value.
    @param  fn  INI file name.
    
    @return
    A D3DXVECTOR3 containing a vector value.
*/
{
char         buffer[256];
std::string  vecVal;

  ::GetPrivateProfileString(sec.c_str(),key.c_str(),def.c_str(),buffer,256,fn.c_str());
  vecVal = buffer;
  
  // Split it by commas and whitespace.
std::vector< std::string >  parts = nsl::stsplit_chr(vecVal,", ");

  // Omitted components are 0.
  while(parts.size() < 4)
    parts.push_back("0");
  
  // Transform into a color.
  return (D3DXVECTOR3(lexical_cast< float >(parts[0]),
                      lexical_cast< float >(parts[1]),
                      lexical_cast< float >(parts[2])));
}


