/* =========================================================================
   
    @file    GraphicsPrimitive.h
    @author  jmp
    @brief   Interface to graphics primative base class.
    
   ========================================================================= */

/*                                                                     guard
---------------------------------------------------------------------------- */

#ifndef _GRAPHICS_PRIMITIVE_H_
#define _GRAPHICS_PRIMITIVE_H_

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsRenderer.h"

namespace Graphics
{


/*                                                                   classes
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
class Primitive
/*! 
*/
{
  public:
    // ct and dt
    Primitive(Renderer *r);
    ~Primitive(void);
    
    // device objects
    virtual void RestoreDeviceObjects(void) = 0;
    virtual void ReleaseDeviceObjects(void) = 0;
    
  protected:
    // typedefs
    typedef std::map< int,LPD3DXEFFECT >  ShaderMap;
  
    // shader compiler
    void nCompileShader(const std::string &fn,LPD3DXEFFECT &shader);
    
    // data members
    Renderer *mRenderer;   //!< Owning renderer object.
    bool      mCanVShade;  //!< Set if vertex shaders should be used.
    bool      mCanPShade;  //!< Set if pixel shaders should be used.
  
    static ShaderMap  smShaderMap;  //!< Shaders for this primitive.
};


}       /* namespace Graphics */
#endif  /* _GRAPHICS_PRIMITIVE_H_ */