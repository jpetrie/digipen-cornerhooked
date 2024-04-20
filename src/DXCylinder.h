/*! ========================================================================

      @file    DXCylinder.h
      @author  jmp
      @brief   Interface to DXCylinder.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_CYLINDER_H_
#define _DX_CYLINDER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class DXCylinder : public Graphics::Primitive
/*! A cylinder rendering object.
*/
{
  public:
    // ct and dt
    DXCylinder(Graphics::Renderer *renderer,float radius,float length,unsigned int r = 16,unsigned int s = 5);
    ~DXCylinder(void);
   
    // rendering
    virtual void Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &eyeVec);
    
    // shader selection
    void UseShader(const std::string &sh) { mShaderTech = sh; };
    
    // device objects
    void RestoreDeviceObjects(void);
    void ReleaseDeviceObjects(void);
    
     // batch render state
    static void EnterRenderState(void);
    static void LeaveRenderState(void);
    
    // transformations
    void SetTranslation(float x,float y,float z);
    void SetRotation(float x,float y,float z,float w);
    void SetScale(float x,float y,float z);
    
    // texture and material management
    void SetTexture(const std::string &tn);
    void SetMaterial(D3DCOLORVALUE ambient,D3DCOLORVALUE diffuse,D3DCOLORVALUE specular,D3DCOLORVALUE emissive,float power);
    
    
  private:
    // structs
	  struct Vertex
	  //! Represents a cylinder vertex.
	  {
	    // data
		  D3DXVECTOR3  pos;   //!< Coordinates.
		  D3DXVECTOR3  norm;  //!< Normal.
	  };
    
    // utility
    void nFillBuffers(void);
  
    // data members  
    float         mRadius;
    float         mLength;
    
    unsigned int  mRes;       //!< 
    unsigned int  mSegments;  //!< Number of segments.
    
    DWORD  mNumVerts;  //!< Total number of vertices.
    DWORD  mNumIndxs;  //!< Total number of indices.
    DWORD  mNumPolys;  //!< Total number of polygons.
    
    LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;    //!< Cylinder vertices.
    LPDIRECT3DINDEXBUFFER9   mIdxBuffer;    //!< Cylinder indices.
    LPDIRECT3DTEXTURE9       mTexture;      //!< Cylinder texture.
    std::string              mTextureName;  //!< Cylinder texture name.
    
    std::string              mShaderTech;
    
    D3DXMATRIX  mMatTranslate;  //!< Translation matrix.
    D3DXMATRIX  mMatScale;      //!< Scaling matrix.
    D3DXMATRIX  mMatRotate;     //!< Rotation matrix.
    
    static LPD3DXEFFECT                  smShader;   //!< Shader for sphere rendering.
	  static LPDIRECT3DVERTEXDECLARATION9  smDecl;     //!< Vertex format declaration.
	  static LPDIRECT3DCUBETEXTURE9        smCubemap;  //!< Cubic environment map.
	  
    static LPDIRECT3DSTATEBLOCK9  smRenderStateBlock;  //!< Stateblock for rendering.
    static bool                   smInRenderState;
    
    static bool                   smValidSharedData;   //!< If true, shared data is valid.
};

#endif  /* _DX_CYLINDER_H_ */