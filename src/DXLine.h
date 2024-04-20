/*! ========================================================================

      @file    DXLine.h
      @author  jmp
      @brief   Interface to DXLine.
      
      (c) 2004 DigiPen (USA) Corporation, all rights reserved.
      
    ======================================================================== */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _DX_LINE_H_
#define _DX_LINE_H_


/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"
#include "GraphicsPrimitive.h"


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class DXLine : public Graphics::Primitive
/*! A line rendering object.
*/
{
  public:
    // ct and dt
    DXLine(Graphics::Renderer *renderer,DWORD start,DWORD stop);
    ~DXLine(void);

    // device objects
    void RestoreDeviceObjects(void);
    void ReleaseDeviceObjects(void);
    
    // batch render state
    static void EnterRenderState(void);
    static void LeaveRenderState(void);
    
    // rendering
	  void DrawLine(float x0,float y0,float z0,float x1,float y1,float z1);
	
	//private:
	  // structs
    struct Vertex
    //! Represents a line vertex.
    {
      // format
      enum { Format = D3DFVF_XYZ | D3DFVF_DIFFUSE };
      
      // data
      D3DXVECTOR3  pos;  //!< Coordinates.
      DWORD        c;    //!< Color.
    };  
    
    // data members
	  LPDIRECT3DVERTEXBUFFER9  mVtxBuffer;  //!< Vertex buffer used to render.
	  DWORD                    mColor0;
	  DWORD                    mColor1;
	
    static LPDIRECT3DSTATEBLOCK9  smRenderStateBlock;  //!< Stateblock for rendering.
    static bool                   smInRenderState;     //!< If true, render state is active.
    
    static bool                   smSharedValid;       //!< If true, shared data is valid.
};


#endif  /* _DX_LINE_H_ */

