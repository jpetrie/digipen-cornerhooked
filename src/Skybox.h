/*! ========================================================================

      @file    Skybox.h
      @author  jmp
      @brief   Interface to skybox object.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _SKYBOX_H_
#define _SKYBOX_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

// texture names
const std::string  kSkyFront  = "front.JPG";
const std::string  kSkyBack   = "back.JPG";
const std::string  kSkyLeft   = "left.JPG";
const std::string  kSkyRight  = "right.JPG";
const std::string  kSkyTop    = "top.JPG";
const std::string  kSkyBottom = "bottom.JPG";
const std::string  kSkyCube   = "cube.dds";
const std::string  kSkyINI    = "skybox.ini";

// skybox times array
const std::string  kSkyStates[] = { "dawn",
                                    "day",
                                    "dusk" };
const int          kSkyStateCnt = 3;

// shader constants
const int  kSkyboxShader = 0;


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Skybox : public Graphics::Primitive
/*! A skybox object.

    The skybox holds, in addition to the planes of the skybox proper,
    information about which type was loaded (dawn/day/dusk/etc.), matching
    UI color information, and environmental information such as ambient light
    color and sun vector.
    
    Only one skybox should ever be created, but this is not enforced (e.g.,
    by conformance to a singleton or monostate pattern).
*/
{
  public:
    // ct and dt
	  Skybox(Graphics::Renderer *renderer,int hours);
	  ~Skybox(void);
   
    // rendering
    virtual void Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &eyeVec);
    
    // device objects
    virtual void ReleaseDeviceObjects(void);
    virtual void RestoreDeviceObjects(void);
    
    // sky state
    static int         GetSkyState(void)    { return (smSkyState); }
    static D3DXVECTOR3 GetSunVector(void)   { return (smSunVector); }
    static DWORD       GetSunColor(void)    { return (smSunColor); }
    static DWORD       UIBase(void)         { return (smUIBaseColor); }
    static DWORD       UIText(void)         { return (smUITextColor); }
    static DWORD       UIDisableText(void)  { return (smUIDisableTextColor); }
    static DWORD       UILabelText(void)    { return (smUILabelTextColor); }
    static DWORD       UIPanelText(void)    { return (smUIPanelColor); }

  private:
    // structs
    struct Vertex
    {
	    D3DXVECTOR3 pos;  //!< Vertex coordinates.
	    D3DXVECTOR2 tex;  //!< Texture coordinates.
    };
    
    // utility
    void nFillVtxBuffer(void);
    
    // ini vector reading
    DWORD       nReadColorVector(const std::string &sec,const std::string &key,const std::string &def,const std::string &fn);
	  D3DXVECTOR3 nReadLightVector(const std::string &sec,const std::string &key,const std::string &def,const std::string &fn);
	
	  // data members
	  static int          smSkyState;            //!< Index into skystate array.
	  static D3DXVECTOR3  smSunVector;           //!< Sunlight vector.
	  static DWORD        smSunColor;            //!< Sunlight color.
	  static DWORD        smUIBaseColor;         //!< Base color.
	  static DWORD        smUITextColor;         //!< Text color.
	  static DWORD        smUIDisableTextColor;  //!< Disabled element text color.
	  static DWORD        smUILabelTextColor;    //!< Label text color.
	  static DWORD        smUIPanelColor;        //!< Panel background color.
	  
	  LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;        //!< The vertex buffer.
	  D3DXVECTOR3              mNormals[6];
	  LPDIRECT3DTEXTURE9       mTextures[6];      //!< The textures.
	  std::string              mTextureNames[6];  //!< Names of the texture files.
	  
	  LPD3DXEFFECT                  mShader;            //!< Active shader.
	  LPDIRECT3DVERTEXDECLARATION9  mDecl;              //!< Shader stream declaration.
	  LPDIRECT3DSTATEBLOCK9         mRenderStateBlock;  //!< Stateblock for rendering.
};

#endif  /* _SKYBOX_H_ */