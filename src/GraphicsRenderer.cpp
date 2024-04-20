/*! ========================================================================
    
      @file   Graphics.cpp
      @author jmc,jmp
      @brief  Implementation of graphics Renderer.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"

namespace Graphics
{

/*                                                                 constants
---------------------------------------------------------------------------- */

namespace
{
  const DWORD  kColorKeyValue= 0xFFFF00FF;
}


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Renderer::Renderer(HWND wind,D3DFORMAT displayFmt,D3DFORMAT depthFmt)
/*! Constructor.

    @param wind        The window to attach the Renderer to.
    @param displayFmt  Format required for display.
    @param backFmt     Format required for backbuffer.
    @param depthFmt    Format required for depthbuffer.
*/
: mWind(wind),
  mD3DObject(0),mD3DDevice(0),
  mAdapterID(D3DADAPTER_DEFAULT),
  mDisplayFmt(displayFmt),mDepthbufferFmt(depthFmt),
  mRenderAllowed(true)
{
	// Setup clear values.
  SetClearColor(0,0,0,255);
  SetClearDepth(1.0f);	
	
	// Create the D3D interface object.
	mD3DObject = ::Direct3DCreate9(D3D_SDK_VERSION);
  ENFORCE(0 != mD3DObject)
         ("Failed to initialize D3D.");
  
  mD3DObject->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&mCaps);
  if((mCaps.VertexShaderVersion & 0xFFFF) < 0x0101 ||
     (mCaps.PixelShaderVersion & 0xFFFF) < 0x0101)
  {
   // br    testing
	 // SAFE_RELEASE(mD3DObject);
   // ::MessageBox(0,"This game cannot run without VS1.1 and PS1.1 or greater support. Sorry.","Error",MB_OK);
   // exit(0);
  }
  
  // Get the supported display modes.
  nEnumDisplayModes();
}

/*  ________________________________________________________________________ */
Renderer::~Renderer(void)
/*! Destructor.
*/
{
  SAFE_RELEASE(mD3DDevice);
	SAFE_RELEASE(mD3DObject);
}

/*  ________________________________________________________________________ */
bool Renderer::CanRender(void)
/*! Test if the renderer's device is usable.

    If the device has been lost and needs to be reset, this function will
    correctly reset it.
    
    @return
    True if the device can be rendered to, false otherwise.
*/
{
  // Are we allowed to be rendering?
  if(!mRenderAllowed)
    return (false);
    
  switch(mD3DDevice->TestCooperativeLevel())
  {
    // Device is lost and cannot be used (e.g., we're tabbed out or something).
    case D3DERR_DEVICELOST:
    {
      return (false);
    }
    
    // Device needs reset.      
    case D3DERR_DEVICENOTRESET:
    {
      // Reset() will fail unless we release all resources allocated 
      // in the default pool (D3DPOOL_DEFAULT).
      nReleasePrimitives();
      ENFORCE_DX(mD3DDevice->Reset(&mD3DParams))
                ("Failed to reset device.");
      nRestorePrimitives();
      return (true);
    }
    
    // All other cases, we're good to go.
    default:
    {
      return (true);
    }
  }
}

/*  ________________________________________________________________________ */
void Renderer::Clear(void)
/*! Clear the framebuffer.
*/
{
int  buffers = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER;

  ASSERT(0 != mD3DDevice);
  ENFORCE_DX(mD3DDevice->Clear(0,0,buffers,mClearColor,mClearDepth,0))
            ("Failed to clear scene.");
}

/*  ________________________________________________________________________ */
void Renderer::Present(void)
/*! Swap framebuffer to the screen.
*/
{
  ASSERT(0 != mD3DDevice);
	ENFORCE_DX(mD3DDevice->Present(0,0,0,0))
	          ("Failed to present scene.");
}


/*  ________________________________________________________________________ */
void Renderer::BeginRender(void)
/*! Begin a render pass.
*/
{
  ASSERT(0 != mD3DDevice);
  ENFORCE_DX(mD3DDevice->BeginScene())
            ("Failed to begin render pass.");
}

/*  ________________________________________________________________________ */
void Renderer::EndRender(void)
/*! End 3D render pass.
*/
{
  ASSERT(0 != mD3DDevice);
	ENFORCE_DX(mD3DDevice->EndScene())
	          ("Failed to end render pass.");
}

/*  ________________________________________________________________________ */
void Renderer::TakeScreenshot(void)
/*! Save a screenshot of the current framebuffer.
*/
{
  ASSERT(0 != mD3DDevice);
  
LPDIRECT3DSURFACE9  surface = 0;
D3DVIEWPORT9        vp;

  // Read from the framebuffer into a in-memory surface.
  ENFORCE_DX(mD3DDevice->GetViewport(&vp))
            ("Failed to get viewport information.");
  ENFORCE_DX(mD3DDevice->CreateOffscreenPlainSurface(vp.Width,vp.Height,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&surface,0))
            ("Failed to create screenshot surface.");
	ENFORCE_DX(mD3DDevice->GetFrontBufferData(0,surface))
	          ("Failed to read framebuffer.");

  // Then write the surface to a file.
  ENFORCE_DX(D3DXSaveSurfaceToFile("screenshot.bmp",D3DXIFF_BMP,surface,0,0))
            ("Failed to save screenshot.");
  SAFE_RELEASE(surface);
}

/*  ________________________________________________________________________ */
LPDIRECT3DTEXTURE9 Renderer::GetTexture(const std::string &tn)
/*! Get a texture.
 
    If the texture was never previously requested, it is loaded from the disk,
    otherwise the cached texture is returned.
    
    @param tn  The name (and optionally, path to) the texture.
    
    @return
    A pointer to the texture, or null if the texture does not exist.
*/
{
FileToTextureMap::iterator  it = mTextures.find(tn);

  if(it == mTextures.end())
  {
  LPDIRECT3DTEXTURE9  tex = nLoadTexture(tn);
  	
  	// Doesn't exist?
  	if(0 == tex)
  	  return (0);
  	
  	// Store and return.
	  mTextures[tn] = tex;
	  return (tex);
  }
	
	// Found it cached, so return it.
	return (it->second);
}

/*  ________________________________________________________________________ */
void Renderer::SetViewport(int x,int y,int w,int h,float n,float f)
/*! Define the viewport.
 
    @param x  X coordinate of the viewport.
    @param y  Y coordinate of the viewport.
    @param w  Width of the viewport.
    @param h  Height of the viewport.
    @param n  Minimum Z value (usually 0.0f).
    @param f  Maximum Z value (usually 1.0f).
*/
{
	mViewport.X      = x;
	mViewport.Y      = y;
	mViewport.Width  = w;
	mViewport.Height = h;
	mViewport.MinZ   = n;
	mViewport.MaxZ   = f;

  ENFORCE_DX(mD3DDevice->SetViewport(&mViewport))
            ("Failed to set viewport.");
}

/*  ________________________________________________________________________ */
void Renderer::SetWorld(D3DXMATRIX &mat)
/*! Set the world transformation matrix.

    @param mat  The matrix to use.
*/
{
	ENFORCE_DX(mD3DDevice->SetTransform(D3DTS_WORLD,&mat))
	          ("Failed to set world matrix.");
}

/*  ________________________________________________________________________ */
void Renderer::SetView(D3DXMATRIX &mat)
/*! Set the viewing transformation matrix.

    @param mat  The matrix to use.
*/
{
  ENFORCE_DX(mD3DDevice->SetTransform(D3DTS_VIEW,&mat))
            ("Failed to set view matrix.");
}

/*  ________________________________________________________________________ */
void Renderer::SetProjection(D3DXMATRIX &mat)
/*! Set the projection matrix.

    @param mat  The matrix to use.
*/
{
  ENFORCE_DX(mD3DDevice->SetTransform(D3DTS_PROJECTION,&mat))
            ("Failed to set projection matrix.");
}

/*  ________________________________________________________________________ */
bool Renderer::SetDisplayMode(unsigned int x,unsigned int y,unsigned int refresh,bool fullscreen)
/*! Set the display mode.

    If fullscreen is true, this function will use the first available
    display mode that matches the input parameters.
    
    @param x           Desired X resolution. Ignored if windowed.
    @param y           Desired Y resolution. Ignored if windowed.
    @param refresh     Desired refresh rate. Ignored if windowed.
                       If zero, the first available is chosen.
    @param fullscreen  If true, use fullscreen display.
    
    @return
    True if the display mode was successfully set, false otherwise.
*/
{
 ASSERT(0 != mD3DObject);
 
  
  ///@todo TEH HAX OMG    and fix the refresh rate issue a little bit down while your at it, bitch
D3DDISPLAYMODE curMode;
  ENFORCE_DX(mD3DObject->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&curMode));
  if(0 == refresh) refresh = curMode.RefreshRate;
  
  // Set up device parameters.
  ::ZeroMemory(&mD3DParams,sizeof(mD3DParams));

  // Fullscreen or windowed?
  mD3DParams.Windowed = !fullscreen;
  if(mD3DParams.Windowed)
  {
    // In windowed mode, only the current display mode is legal.
		mD3DParams.BackBufferWidth  = x;
		mD3DParams.BackBufferHeight = y;
		mD3DParams.BackBufferFormat = D3DFMT_UNKNOWN;
  }
  else
  {
  bool                       legal = false;
  DisplayModeList::iterator  it    = mDisplayModes.begin();
  
    // Search for a matching display format.
    while(it != mDisplayModes.end())
    {
      if(it->Width == x && it->Height == y)
      {
        // Are we requiring refresh rate as well?
        if(refresh != 0)///@todo i think i need to do all this a bit better!
        {
          if(it->RefreshRate == refresh)
          {
            legal = true;
            break;
          }
        }
        else
        {
          legal = true;
          break;
        }
      }
      ++it;
    } 
    
    // If no legal modes, choke.
    if(!legal)
      return (false);
    
		mD3DParams.BackBufferWidth            = it->Width;
		mD3DParams.BackBufferHeight           = it->Height;
		mD3DParams.BackBufferFormat           = it->Format;
		mD3DParams.FullScreen_RefreshRateInHz = it->RefreshRate;
  }
  
  mD3DParams.EnableAutoDepthStencil = true;
	mD3DParams.AutoDepthStencilFormat = mDepthbufferFmt;
	mD3DParams.BackBufferCount        = 1;
	mD3DParams.MultiSampleType        = D3DMULTISAMPLE_NONE;
	mD3DParams.MultiSampleQuality     = 0;
	mD3DParams.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	mD3DParams.hDeviceWindow          = mWind;
	mD3DParams.Flags                  = 0;
	mD3DParams.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

  // See if we can use HAL.
  mDevType = D3DDEVTYPE_HAL;
  if(FAILED(mD3DObject->CheckDeviceType(mAdapterID,D3DDEVTYPE_HAL,
                                        mDisplayFmt,
                                        mD3DParams.BackBufferFormat,
                                        mD3DParams.Windowed)))
  {
    // Try REF.
    mDevType = D3DDEVTYPE_REF;
    if(FAILED(mD3DObject->CheckDeviceType(mAdapterID,D3DDEVTYPE_REF,
                                          mDisplayFmt,
                                          mD3DParams.BackBufferFormat,
                                          mD3DParams.Windowed)))
    {
      // Bad news. Error out.
      return (false);
    }
  }
 
  // Try to create with hardware vertex processing.
  mVtxProcType = D3DCREATE_HARDWARE_VERTEXPROCESSING;
  if(FAILED(mD3DObject->CreateDevice(D3DADAPTER_DEFAULT,mDevType,mWind,mVtxProcType,&mD3DParams,&mD3DDevice)))
  {
    // If that failed, try software.
    mVtxProcType = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    if(FAILED(mD3DObject->CreateDevice(D3DADAPTER_DEFAULT,mDevType,mWind,mVtxProcType,&mD3DParams,&mD3DDevice)))
    {
      // Error out.
      return (false);
    }
  }
	
  // Update device caps.
  mD3DDevice->GetDeviceCaps(&mCaps);
  return (true);
}

/*  ________________________________________________________________________ */
void Renderer::nEnumDisplayModes(void)
/*! Enumerate the available display modes.
*/
{
 ASSERT(0 != mD3DObject);

UINT            modeCnt = mD3DObject->GetAdapterModeCount(mAdapterID,mDisplayFmt);
D3DDISPLAYMODE  mode;  

  // Enumerate available modes.
  for(UINT modeIdx = 0; modeIdx < modeCnt; ++modeIdx)
  {
    // If the adapter supports the mode, store it.
    if(!FAILED(mD3DObject->EnumAdapterModes(mAdapterID,mDisplayFmt,modeIdx,&mode)))
      mDisplayModes.push_back(mode);
  }
}

/*  ________________________________________________________________________ */
void Renderer::InsertPrimitive(Primitive *p)
/*! Insert a primitive into the renderer's list.

    @param p  The primitive to insert.
*/
{
  mPrimitives.insert(p);
}

/*  ________________________________________________________________________ */
void Renderer::RemovePrimitive(Primitive *p)
/*! Rempve a primitive from the renderer's list.

    @param p  The primitive to remove.
*/
{
PrimitiveList::iterator  it = mPrimitives.find(p);

  if(it != mPrimitives.end())
    mPrimitives.erase(it);
}

/*  ________________________________________________________________________ */
void Renderer::nReleasePrimitives(void)
/*! Invoke ReleaseDeviceObjects() for every attached primitive.

    This function will also release the cached texture data.
*/
{
PrimitiveList::iterator    it  = mPrimitives.begin();
FileToTextureMap::iterator tex = mTextures.begin();

  while(it != mPrimitives.end())
  {
    (*it)->ReleaseDeviceObjects();
    ++it;
  }
  
  // We also need to free the textures (but save the names so we can restore
  // them in the coming call to nRestorePrimitives()).
  while(tex != mTextures.end())
  {
    SAFE_RELEASE(tex->second);
    ++tex;
  }
}

/*  ________________________________________________________________________ */
void Renderer::nRestorePrimitives(void)
/*! Invoke RestoreDeviceObjects() for every attached primitive.

    This function will also restore the cached texture data.
*/
{
PrimitiveList::iterator    it  = mPrimitives.begin();
FileToTextureMap::iterator tex = mTextures.begin();

  // Restore the textures first, since some primitives may depend on them.
  while(tex != mTextures.end())
  {
  LPDIRECT3DTEXTURE9  texture;
  
    // Restore it.
    texture = nLoadTexture(tex->first.c_str());
    ENFORCE(0 != texture)
           ("Failed to restore texture: ")(tex->first.c_str());
    
    // Copy and move on.
    tex->second = texture;
    ++tex;
  }
  
  // Now do the primitives.
  while(it != mPrimitives.end())
  {
    (*it)->RestoreDeviceObjects();
    ++it;
  } 
}

/*  ________________________________________________________________________ */
LPDIRECT3DTEXTURE9 Renderer::nLoadTexture(const std::string &tn)
/*! Load a texture file.

    @param tn  The name of the texture to load.
    
    @return
    A pointer to the loaded texture, or null if the texture was not found.
*/
{
D3DFORMAT           fmt;
LPDIRECT3DTEXTURE9  tex;

  // Stupid-proofing.
  if(tn.empty())
    return (0);

  // Use preferred format, if possible.
	if(mD3DParams.BackBufferFormat == D3DFMT_X8R8G8B8 )
		fmt = D3DFMT_A8R8G8B8;
	else
		fmt = mD3DParams.BackBufferFormat;

  // Try and create it.
  if(FAILED(D3DXCreateTextureFromFileEx(mD3DDevice,tn.c_str(),0,0,0,0,fmt,
                                        D3DPOOL_DEFAULT,D3DX_FILTER_NONE,D3DX_FILTER_TRIANGLE,
                                        kColorKeyValue,0,0,&tex)))
  {
    return (0);
  }
  return (tex);
}

}  /* namespace Graphics */