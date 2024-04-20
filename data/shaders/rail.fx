/* =========================================================================
   
    @file    rail.fx
    @author  jmp
    @brief   Rail cylinder shader.
    
   ========================================================================= */

/*                                                                 variables
---------------------------------------------------------------------------- */

uniform float4  gEye;       // Position (world space) of the camera.
uniform float4  gLight;     // Vector from light source, world space.

float4x4        gMatW;      // World transform matrix.
float4x4        gMatWN;     // inverse(transpose(gMatW)), for normal transforms.
float4x4        gMatVP;     // View and projection matrix.

textureCUBE     gEnvirons;  // Texture to use as environment map.

/*                                                                  samplers
---------------------------------------------------------------------------- */

samplerCUBE sampEnvirons = sampler_state
{
  Texture   = <gEnvirons>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};


/*                                                                   structs
---------------------------------------------------------------------------- */

struct VOUT
{
  float4 pos   : POSITION;
  float3 refl  : TEXCOORD0;
  float4 diff  : COLOR;
};

struct POUT
{
	float4 color  : COLOR;
};


/*                                                                   shaders
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
VOUT VSMain(float3 pos    : POSITION,
            float3 normal : NORMAL)
/*! Vertex shader.
*/
{
VOUT    o;
float4  p;
float3  n;
float4  l;

  // Transform the vertex and normal into world space.
  p = mul(float4(pos,1.0),gMatW);
  n = mul(float4(normal,1.0),gMatWN);
  n = normalize(n);
  
  // Normalize the light vector.
  l = normalize(gLight);
  
  // Write diffuse color.
  o.diff = dot(n,l);
  
  // Compute a vector from the vertex to the eye.
float4  v = gEye - p;

  v = normalize(v);

  // Compute the reflection vector.
float3 ref = reflect(v,n);

  // And write it.
  o.refl = normalize(ref);

  // Finish vertex transformation.
  o.pos = mul(p,gMatVP);
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSMain(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;

  // Sample texture and write.
  o.color   = p.diff * texCUBE(sampEnvirons,p.refl);
  o.color.a = 1.0f;
  return (o);
}


/*                                                                techniques
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
technique RailV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSMain();
  }
}
