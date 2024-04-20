/* =========================================================================
   
    @file    GraphicsPrimitive.cpp
    @author  jmp
    @brief   Implementation of graphics primative base class.
    
   ========================================================================= */

/*                                                                  includes
---------------------------------------------------------------------------- */

#include "main.h"

#include "GraphicsPrimitive.h"

namespace Graphics
{


/*                                                                 variables
---------------------------------------------------------------------------- */

// shader map
Primitive::ShaderMap  Primitive::smShaderMap;


/*                                                                 functions
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
Primitive::Primitive(Renderer *r)
/*! Constructor.
    
    @param r  Renderer to attach the primitive to.
*/
: mRenderer(r),
  mCanVShade(false),mCanPShade(false)
{
  if(r->GetCaps().VertexShaderVersion >= 0x0101)
    mCanVShade = true;
  if(r->GetCaps().PixelShaderVersion >= 0x0101)
    mCanPShade = true;

  mRenderer->InsertPrimitive(this);
}

/*  ________________________________________________________________________ */
Primitive::~Primitive(void)
/*! Destructor.
*/
{
  mRenderer->RemovePrimitive(this);
}

/*  ________________________________________________________________________ */
void Primitive::nCompileShader(const std::string &fn,LPD3DXEFFECT &shader)
/*! Utility for performing shader compilation.

    This function will do nothing if shaders are not supported; otherwise
    it will compile the given effect file into an effect object and store
    it in the primitive's shader list.
    
    @param fn      Name of the effect file to compile.
    @param key     Shader key (ID).
*/
{
  // If shader support exists, compile shaders.
  if(mCanVShade)
  {
  LPD3DXBUFFER  errs = 0;
  DWORD         flags = 0;
       
    // In debug mode, shaders should be compiled with symbol information
    // and other goodies.
#ifdef _DEBUG
  flags = D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
#endif
   
    try
    {
      ENFORCE_DX(D3DXCreateEffectFromFile(mRenderer->GetDevice(),fn.c_str(),0,0,flags,0,&shader,&errs))
                ("Failed to compile shader: ")(fn.c_str());
                
      // Put the shader in the map. Key reuse causes existing shader with
      // that key to be release.
   /*   if(smShaderMap.find(key) != smShaderMap.end())
        smShaderMap[key]->Release();
      smShaderMap.insert(std::make_pair(key,shader));*/
    }
    catch(...)
    {
      // If the shader compilation fails, we want to abort. However,
      // if we're also in debug mode, it'd be nice to see the reason
      // we couldn't compile.
#ifdef _DEBUG
    LPVOID  buf = errs->GetBufferPointer();
    
      ::MessageBox(0,reinterpret_cast< char* >(buf),"Compile Error",MB_OK);
#endif
      // Re-throw exception.
      throw;
    }
  }
}

}  /* namespace Graphics */
 