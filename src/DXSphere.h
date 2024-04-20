/*! ========================================================================

      @file    DXSphere.h
      @author  jmp
      @brief   Interface to DXSphere.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */
    
/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_SPHERE_H_
#define _DX_SPHERE_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"

#include "Clock.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

#define D3DXSPHERE_LOD_LEVELS	8

// limits
const int  kSphereCausticTextures = 32;

// shader names
const std::string kSphereDefaultSh   = "BallV11P11";
const std::string kSphereGhostballSh = "BallGhostV11P11";


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class DXSphere : public Graphics::Primitive
/*! A sphere rendering object.
*/
{
  public:
    // ct and dt
    DXSphere(Graphics::Renderer *renderer,const std::string &tex,unsigned int r = 20,unsigned int s = 20);
    ~DXSphere(void);
   
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
	  //! Represents a sphere vertex.
	  {
	    // data
		  D3DXVECTOR3  pos;   //!< Coordinates.
		  D3DXVECTOR3  norm;  //!< Normal.
		  D3DXVECTOR2  tex;   //!< Texture coordinates.
	  };
    
    // utility
    void nFillBuffers(unsigned int i);
  
    // data members  
    unsigned int  mRings[D3DXSPHERE_LOD_LEVELS];     //!< Number of rings in the sphere.
    unsigned int  mSegments[D3DXSPHERE_LOD_LEVELS];  //!< Number of segments in the sphere.
   
    unsigned int  mCausticID;
    Clock         mCausticClock;
    
    DWORD  mNumVerts[D3DXSPHERE_LOD_LEVELS];  //!< Total number of vertices.
    DWORD  mNumIndxs[D3DXSPHERE_LOD_LEVELS];  //!< Total number of indices.
    DWORD  mNumPolys[D3DXSPHERE_LOD_LEVELS];  //!< Total number of polygons.
    
    LPDIRECT3DVERTEXBUFFER9  mVtxBuffer[D3DXSPHERE_LOD_LEVELS];    //!< Sphere vertices.
    LPDIRECT3DINDEXBUFFER9   mIdxBuffer[D3DXSPHERE_LOD_LEVELS];    //!< Sphere indices.
    LPDIRECT3DTEXTURE9       mTexture;      //!< Sphere texture.
    D3DMATERIAL9             mMaterial;     //!< Sphere material.
    std::string              mTextureName;  //!< Sphere texture name.
    
    std::string              mShaderTech;
    
    D3DXMATRIX  mMatTranslate;  //!< Translation matrix.
    D3DXMATRIX  mMatScale;      //!< Scaling matrix.
    D3DXMATRIX  mMatRotate;     //!< Rotation matrix.
    
    static LPD3DXEFFECT                  smShader;   //!< Shader for sphere rendering.
	  static LPDIRECT3DVERTEXDECLARATION9  smDecl;     //!< Vertex format declaration.
	  static LPDIRECT3DCUBETEXTURE9        smCubemap;  //!< Cubic environment map.
	  
	  static LPDIRECT3DTEXTURE9            smCaustics[kSphereCausticTextures];
	  
    static LPDIRECT3DSTATEBLOCK9  smRenderStateBlock;  //!< Stateblock for rendering.
    static bool                   smInRenderState;
    
    static bool                   smValidSharedData;   //!< If true, shared data is valid.
};

#endif  /* _DX_SPHERE_H_ */