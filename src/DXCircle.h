/*! ========================================================================

      @file    DXCircle.h
      @author  jmp
      @brief   Interface to DXCircle.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_CIRCLE_H_
#define _DX_CIRCLE_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class DXCircle : public Graphics::Primitive
/*! A circle rendering object.
*/
{
  public:
    // ct and dt
    DXCircle(Graphics::Renderer *renderer,DWORD color,unsigned int resolution);
    ~DXCircle(void);

    // device objects
    void RestoreDeviceObjects(void);
    void ReleaseDeviceObjects(void);
    
    // batch render state
    static void EnterRenderState(void);
    static void LeaveRenderState(void);
    
    // rendering
	  void DrawCircle(float x,float y,float z,float r,D3DXVECTOR3 normal);
	
	private:
	  // structs
    struct Vertex
    //! Represents a circle vertex.
    {
      // format
      enum { Format = D3DFVF_XYZ | D3DFVF_DIFFUSE };
      
      // data
      D3DXVECTOR3  pos;  //!< Coordinates.
      DWORD        c;    //!< Color.
    };  
    
    // data members
	  LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;   //!< Vertex buffer used to render.
	  DWORD                    mColor;       //!< Color of the circle.
	  unsigned int             mResolution;  //!< Resolution of the circle.
	
    static LPDIRECT3DSTATEBLOCK9  smRenderStateBlock;  //!< Stateblock for rendering.
    static bool                   smInRenderState;     //!< If true, render state is active.
    
    static bool                   smSharedValid;       //!< If true, shared data is valid.
};


#endif  /* _DX_CIRCLE_H_ */

