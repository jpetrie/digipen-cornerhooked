/* =========================================================================
   
    @file    ui.fx
    @author  jmp
    @brief   UI element shader.
    
   ========================================================================= */

/*                                                                 variables
---------------------------------------------------------------------------- */

uniform float4x4  gMatWVP;   // Concatenated world-view-projection matrix.

texture           gTexture;  // Texture to sample.


/*                                                                  samplers
---------------------------------------------------------------------------- */

sampler sampTexture = sampler_state
{
  Texture   = <gTexture>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};


/*                                                                   structs
---------------------------------------------------------------------------- */

struct VOUT
{
  float4 pos   : POSITION;   //!< Vertex position.
  float4 color : COLOR0;     //!< Color.
  float2 tex0  : TEXCOORD0;  //!< Texture coordinates.
};


struct POUT
{
	float4 color : COLOR;  //!< Color.
};


/*                                                                   shaders
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
VOUT VSMain(float3 pos   : POSITION,
            float4 color : COLOR0,
            float2 tex0  : TEXCOORD0)
/*! Vertex shader.
*/
{
VOUT  o;

  // Transform the vertex.
  o.pos  = mul(float4(pos,1.0),gMatWVP);
  
  // Pass the texture coordinates and color through.
  o.tex0  = tex0;
  o.color = color;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSMainTex(VOUT p)
/*! Pixel shader for textured elements.
*/
{
POUT  o;

  // Sample texture and write.
  o.color = tex2D(sampTexture,p.tex0);
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSMainCol(VOUT p)
/*! Pixel shader for untextured elements.
*/
{
POUT  o;

  // Write color.
  o.color = p.color;
  return (o);
}


/*                                                                techniques
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
technique UITextureV11P11
/*! VS1.1 and PS1.1; textured elements.
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSMainTex();
  }
}

/*  ________________________________________________________________________ */
technique UIColorV11P11
/*! VS1.1 and PS1.1; untextured elements.
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSMainCol();
  }
}