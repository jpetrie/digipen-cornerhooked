/* =========================================================================
   
    @file    pocket.fx
    @author  jmp
    @brief   Pocket shader.
    
   ========================================================================= */

/*                                                                 variables
---------------------------------------------------------------------------- */

float4x4  gMatWVP;   // Concatenated world-view-projection matrix.
texture   gTexture;  // Texture to sample.

/*                                                                  samplers
---------------------------------------------------------------------------- */

sampler sampTexture = sampler_state
{
  Texture   = <gTexture>;
  MinFilter = POINT;
  MagFilter = POINT;
  MipFilter = POINT;
};


/*                                                                   structs
---------------------------------------------------------------------------- */

struct VOUT
{
  float4 pos   : POSITION;
  float2 tex0  : TEXCOORD0;
};


struct POUT
{
	float4 color : COLOR;
};


/*                                                                   shaders
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
VOUT VSMain(float3 pos  : POSITION,
            float2 tex0 : TEXCOORD0)
/*! Vertex shader.
*/
{
VOUT  o;

  // Transform the vertex.
  o.pos  = mul(gMatWVP,float4(pos,1.0));
  
  // Pass the texture coordinates through.
  o.tex0 = tex0;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSMain(VOUT p)
/*! Pixel shader.
*/
{
POUT  o;

  // Sample texture and write.
  o.color = tex2D(sampTexture,p.tex0);
  return (o);
}


/*                                                                techniques
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
technique SkyboxV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSMain();
  }
}