/*! ========================================================================

      @file    DXRect.h
      @author  jmp
      @brief   Interface to DXRect.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_RECT_H_
#define _DX_RECT_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class DXRect : public Graphics::Primitive
/*! A rectangle rendering object.
*/
{
  public:
    // ct and dt
    DXRect(Graphics::Renderer *renderer,DWORD tl,DWORD tr,DWORD bl,DWORD br,const std::string &tex = "");
    ~DXRect(void);

    // device objects
    void RestoreDeviceObjects(void);
    void ReleaseDeviceObjects(void);
    
    // batch render state
    static void EnterRenderState(void);
    static void LeaveRenderState(void);
    
    // rendering
	  void DrawRect(int x,int y,int w,int h);
	
	  // texture control
	  void SetTexture(const std::string &tex);
	
//	private:	
	  // structs
    struct Vertex
    //! Represents a rectangle vertex.
    {
      // data
      D3DXVECTOR3  pos;  //!< Coordinates. Z should be zero.
      DWORD        c;    //!< Color.
      float        u,v;  //!< Texture coordinates.
    };  

    // data members
	  LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;  //!< Vertex buffer used to render.
	  LPDIRECT3DTEXTURE9       mTexture;    //!< Texture used to render.
    DWORD                    mColors[4];  //!< Top left and right, bottom left and right.
    
	  std::string  mTextureName;  //!< Name of the texture.
    
    static LPD3DXEFFECT                  smShader;
    static LPDIRECT3DVERTEXDECLARATION9  smDecl;              //!< Shader stream declaration.
    static LPDIRECT3DSTATEBLOCK9         smRenderStateBlock;  //!< Stateblock for rendering.
    static bool                          smInRenderState;     //!< If true, render state is active.
    
    static bool                          smSharedValid;       //!< If true, shared data is valid.
};


#endif  /* _DX_RECT_H_ */

