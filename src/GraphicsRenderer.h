/*! ========================================================================
    
      @file   Graphics.h
      @author jmc,jmp
      @brief  Interface to graphics renderer.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _GRAPHICS_RENDERER_H_
#define _GRAPHICS_RENDERER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"


namespace Graphics
{


/*                                                                   classes
---------------------------------------------------------------------------- */

// forward declarations
class Primitive;

/*  ________________________________________________________________________ */
class Renderer
/*! 
*/
{
  public:
    // typedefs
    typedef std::vector< D3DDISPLAYMODE >  DisplayModeList;  //!< List of display modes.
    typedef DisplayModeList::iterator      DisplayModeIter;  //!< Display mode iterator. 
    
    // ct and dt
	  Renderer(HWND wind,D3DFORMAT displayFmt,D3DFORMAT depthFmt);
	  ~Renderer(void);

    // accessors
    bool IsFullscreen(void) const { return (!mD3DParams.Windowed); }

    // components
	  LPDIRECT3DDEVICE9 GetDevice(void) { ASSERT(0 != mD3DDevice); return (mD3DDevice); }	  
    
    // render query
    void CanRender(bool v) { mRenderAllowed = v; }
    bool CanRender(void);
    
    // rendering
    void Clear(void);
    void Present(void);
    void BeginRender(void);
    void EndRender(void);
    
    // screenshot
    void TakeScreenshot(void);

    // clear values
    void SetClearColor(int r,int g,int b,int a) { mClearColor = D3DCOLOR_ARGB(a,r,g,b); }
    void SetClearDepth(float d)                 { mClearDepth = d; }
	
	  // primative management
	  void InsertPrimitive(Primitive *p);
	  void RemovePrimitive(Primitive *p);
	  
	  // texture management
	  LPDIRECT3DTEXTURE9 GetTexture(const std::string &tn);
	
    // transformations
	  void SetViewport(int x,int y,int w,int h,float n,float f);
	  void SetWorld(D3DXMATRIX &mat);
	  void SetView(D3DXMATRIX &mat);
    void SetProjection(D3DXMATRIX &mat);	

    // caps
    const D3DCAPS9& GetCaps(void) const { return (mCaps); }
    
	  // display mode
    bool             SetDisplayMode(unsigned int x,unsigned int y,unsigned int refresh,bool fullscreen);
    DisplayModeIter  ModesBegin(void) { return (mDisplayModes.begin()); }
    DisplayModeIter  ModesEnd(void)   { return (mDisplayModes.end()); }

  private:
    // typedefs
    typedef std::set< Primitive* >                      PrimitiveList;     //!< List of primitive renderables.
    typedef std::map< std::string,LPDIRECT3DTEXTURE9 >  FileToTextureMap;  //!< Maps filenames to textures.
    
    // enumerators
    void nEnumDisplayModes(void);
    
    // primitive management
    void nReleasePrimitives(void);
	  void nRestorePrimitives(void);
	  
	  // texture loader
	  LPDIRECT3DTEXTURE9  nLoadTexture(const std::string &tn);
    
    // data members
    HWND                   mWind;       //!< Window handle.
    LPDIRECT3D9            mD3DObject;  //!< D3D interface object.
    LPDIRECT3DDEVICE9      mD3DDevice;  //!< D3D device object.
	  D3DPRESENT_PARAMETERS  mD3DParams;  //!< Presentation parameters.
	  D3DCAPS9               mCaps;       //!< D3D caps.
	  
	  DWORD  mClearColor;  //!< Color framebuffer is cleared to.
	  float  mClearDepth;  //!< Value depth buffer is cleared to.

    UINT  mAdapterID;            //!< Identifier for the adapter in use by the renderer.
    
    D3DFORMAT   mDisplayFmt;      //!< Required display format.
    D3DFORMAT   mDepthbufferFmt;  //!< Required depthbuffer format;
	  D3DDEVTYPE  mDevType;         //!< HAL or REF device.
	  UINT        mVtxProcType;     //!< Hardware or software vertex processing.
	  
	  D3DVIEWPORT9  mViewport;  //!< Viewport.
	  
	  DisplayModeList   mDisplayModes;  //!< List of available display modes.
	  PrimitiveList     mPrimitives;    //!< List of attached primitives.
	  FileToTextureMap  mTextures;      //!< Mapped textures.
	  
	  bool  mRenderAllowed;  //!< If true, rendering is allowed. 
};

}       /* namespace Graphics */
#endif  /* _GRAPHICS_RENDERER_H_ */