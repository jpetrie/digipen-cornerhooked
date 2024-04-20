/*! ========================================================================

      @file    Water.h
      @author  jmp
      @brief   Interface to water renderer.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */


/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _WATER_H_
#define _WATER_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"

#include "Clock.h"


/*                                                                 constants
---------------------------------------------------------------------------- */

const float  kWaterTimeStep = 0.1f;  // Also used by the balls for caustics.

const int  kWaterNumGeoWaves = 4;
const int  kWaterNumTexWaves = 16;
const int  kWaterBumpPerPass = 4;
const int  kWaterBumpPassCnt = kWaterNumTexWaves / kWaterBumpPerPass;


/*  ________________________________________________________________________ */
class Water : public Graphics::Primitive
/*! 
*/
{
  public:
    // ct and dt
    Water(Graphics::Renderer *renderer,float sx,float sy,unsigned int rx,unsigned int ry);
    ~Water(void);
    
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
    
    // update
    void Update(void);
    
    // disturbance
    void Disturb(float x,float y);
    
    // rendering
    virtual void Render(const D3DXVECTOR3 &eyePos,const D3DXVECTOR3 &eyeVec);
    
    // world normal
    void        SetWorldNormal(const D3DXVECTOR3 &n) { mWorldNormal = n; }
    D3DXVECTOR3 GetWorldNormal(void) const           { return (mWorldNormal); }
    
    bool  isRendered;
    
 // private:
    // structs
    struct Vertex
    //! Represents a water plane vertex.
    {
      // data
      D3DXVECTOR3  pos;       //!< Coordinates.
      D3DXVECTOR3  normal;    //!< Normal.
      D3DXVECTOR2  texc;      //!< Texture coordinates.
    };
    
    struct VertexRTT
    //!< Represents a vertex used during render-to-texture.
    {
      // data
      D3DXVECTOR3  pos;   //!< Coordinates. Z should be 1.0.
      D3DXVECTOR2  texc;  //!< Texture coordinates.
    };

    struct GeoWaveParams
    //! Represents shader parameters for geometry waves.
    {
      D3DXHANDLE  time;     //!< Simulation time.
      D3DXHANDLE  amps;     //!< Wave amplitudes.
      D3DXHANDLE  frqs;     //!< Wave frequencies.
      D3DXHANDLE  spds;     //!< Wave speeds.
      D3DXHANDLE  dirx;     //!< Wave X direction components.
      D3DXHANDLE  diry;     //!< Wave Y direction components.
      D3DXHANDLE  envMap;   //!< Environment map.
      D3DXHANDLE  bumpMap;  //!< Bump map.
    };
    
    struct TexWaveParams
    //! Represents shader parameters for texture waves.
    {
      D3DXHANDLE  trans[kWaterNumTexWaves];  //!< Wave equation translations.
      D3DXHANDLE  coeff[kWaterNumTexWaves];  //!< Wave equation coefficients.
      D3DXHANDLE  rescale;                   //!< Rescaling factor.
      D3DXHANDLE  noiseBias[4];              //!< Noise bias vectors.
      D3DXHANDLE  scaleBias;                 //!< Noise rescaling factor.
    };
    
    struct GeoWaveInfo
    //! Represents information about a geometry wave. 
    {
      float		     amp;          //!< Wave amplitude.
      float		     len;          //!< Wave length.
      float		     frq;          //!< Wave frequency.
      float		     spd;          //!< Wave speed.
      float		     fade;         //!< Wave fade in/out time.
      D3DXVECTOR2	 direction;    //!< Wave direction information.
    };

    struct GeoSimState
    //! Represents global geometry wave simulation data.
    {
      float		     chop;            //!< Wave chop factor.
      float		     angleDeviation;  //!< Angle deviation.
      D3DXVECTOR2  windDir;         //!< Wind direction (principal wave direction).
      float        maxLength;       //!< Maximum wave length.
      float		     minLength;       //!< Minimum wave length.
      float		     ampOverLen;      //!< Required ratio of amplitude to length.
      float        specAtten;
      float        specEnd;
      float        specTrans;
      float        envHeight;
      float        envRadius;
      int		       transIdx;        //!< Which wave is fading.
      float        transDel;        //!< Fade delta.
    };
    
    struct TexWaveInfo
	  //! Represents information about a texture wave.
	  {
		  float		     phase;        //!< Wave phase.
		  float		     amp;          //!< Wave amplitude.
		  float		     len;          //!< Wave length.
		  float		     spd;          //!< Wave speed.
		  float		     frq;          //!< Wave frequency.
		  float		     fade;         //!< Wave fade in/out time.
		  D3DXVECTOR2	 direction;    //!< Wave direction information.
		  D3DXVECTOR2	 rotateScale;  //!< Wave rotatation information.
	  };
	  
	  struct BallWaveInfo
	  //! Represents information about a ball/disturbance wave.
	  {
	    float  time;
	    float  x;
	    float  y;
	    float  force;
	  };

	  struct TexSimState
	  //! Represents global texture wave simulation data.
    {
      float		     noise;           //!< Wave noise factor.
      float		     chop;            //!< Wave chop factor.
      float		     angleDeviation;  //!< Angle deviation.
      D3DXVECTOR2  windDir;         //!< Wind direction (principal wave direction).
      float        maxLength;       //!< Maximum wave length.
      float		     minLength;       //!< Minimum wave length.
      float		     ampOverLen;      //!< Required ratio of amplitude to length.
      float		     rippleScale;     //!< Allowed ripple scale.
      float		     spdDeviation;    //!< Allowed speed deviation.
      int		       transIdx;        //!< Which wave is fading.
      float        transDel;        //!< Fade delta.
    };
    
    // utility
    void nFillBuffers(void);
    void nFillBuffersRTT(void);
    void nFillCosineLUT(void);
    void nFillNoiseLUT(void);
    
    // geometry waves
    void nInitGeoWaves(void);
    void nInitGeoState(void);
    void nInitGeoWave(int i);
    void nUpdateGeoWaves(float t);
    void nUpdateGeoWave(float t,int i);
    void nGetGeoWaveParams(void);
    void nSetGeoWaveParams(void);
    
    // ball waves
    void nUpdateBallWaves(float t);
    void nUpdateBallWave(float t,int i,Vertex *vtx);
    
    // texture waves
    void nInitTexWaves(void);
    void nInitTexState(void);
    void nInitTexWave(int i);
    void nUpdateTexWaves(float t);
    void nUpdateTexWave(float t,int i);
    void nGetTexWaveParams(void);
    void nSetTexWaveParams(void);
    
    // normal map rendering
    void nRenderNormalMap(void);
    
    // data members
    LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;    //!< Water vertices.
    LPDIRECT3DINDEXBUFFER9   mIdxBuffer;    //!< Water indices.
    LPDIRECT3DTEXTURE9       mNormalMap;    //!< Water detail bump map.
    D3DXVECTOR3              mWorldNormal;  //!< Worldspace plane normal.

    GeoSimState  mGeoState;                     //!< Geometry wave simulation state.
    GeoWaveInfo  mGeoWaves[kWaterNumGeoWaves];  //!< Geometry wave individual information.
	  TexSimState  mTexState;                     //!< Texture wave simulation state.
    TexWaveInfo  mTexWaves[kWaterNumTexWaves];  //!< Texture wave individual information.
    
    std::vector< BallWaveInfo >  mBallWaves;
    
    Clock                      mWaveSimClock;  //!< Clock for controlling wave simulation.
    float                      mWaveSimTime;   //!< Time of wave simulation.
    
    float         mSX;         //!< X size.
    float         mSY;         //!< Y size.
    unsigned int  mRX;         //!< X resolution.
    unsigned int  mRY;         //!< Y resolution.
    unsigned int  mNumVerts;   //!< Number of vertices.
    unsigned int  mNumIndxs;   //!< Number of indices.
    unsigned int  mNumStrips;  //!< Number of triangle strips.
    
    D3DXMATRIX  mMatTranslate;  //!< Translation matrix.
    D3DXMATRIX  mMatScale;      //!< Scaling matrix.
    D3DXMATRIX  mMatRotate;     //!< Rotation matrix.
    
    static GeoWaveParams  smGeoWaveParams;  //!< Geometry wave parameter handles.
    static TexWaveParams  smTexWaveParams;  //!< Texture wave parameter handles.
    
    static LPD3DXEFFECT                  smShader;    //!< Shader object.
    static LPDIRECT3DVERTEXDECLARATION9  smDecl;      //!< Vertex format declaration.
    static LPDIRECT3DVERTEXDECLARATION9  smDeclRTT;   //!< Vertex format for RTT.
    static LPDIRECT3DCUBETEXTURE9        smCubemap;   //!< Cubic environment map.
    static LPDIRECT3DTEXTURE9            smCosLUT;    //!< Cosine lookup table texture.
    static LPDIRECT3DTEXTURE9            smNoiseLUT;  //!< Noise lookup table texture.
    static LPDIRECT3DVERTEXBUFFER9       smRTTVB;     //!< Vertex buffer for render-to-texture.
    
    static LPDIRECT3DSTATEBLOCK9  smRenderStateBlock;  //!< Stateblock for rendering.
    static bool                   smInRenderState;     //!< If true, in batch render state.
    static bool                   smValidSharedData;   //!< If true, shared data is valid.
};


#endif  /* _WATER_H_ */