/* =========================================================================
   
    @file    ball.fx
    @author  jmp
    @brief   Ball shader.
    
   ========================================================================= */

/*                                                                 variables
---------------------------------------------------------------------------- */

uniform float4  gEye;       // Position (world space) of the camera.
uniform float4  gLight;     // Vector from light source, world space.
uniform float4  gAmbient;   // Ambient (sun) color. RGBA.

float4x4        gMatW;      // World transform matrix.
float4x4        gMatWN;     // inverse(transpose(gMatW)), for normal transforms.
float4x4        gMatVP;     // View and projection matrix.

texture         gTexture;   // Texture to sample.
texture         gCaustic;   // Texture to sample for caustic effect.
textureCUBE     gEnvirons;  // Texture to use as environment map.


/*                                                                  samplers
---------------------------------------------------------------------------- */

sampler sampTexture = sampler_state
{
  Texture   = <gTexture>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};

sampler sampCaustic = sampler_state
{
  Texture   = <gCaustic>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};

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
  float2 tex0  : TEXCOORD0;
  float3 tex1  : TEXCOORD1;
  float2 tex2  : TEXCOORD2;
  float3 view  : TEXCOORD3;
  float4 diff  : COLOR0;
};

struct VROUT
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
            float3 normal : NORMAL,
            float2 tex0   : TEXCOORD0)
/*! Vertex shader.
*/
{
VOUT    o;
float4  p;
float3  n;

  // Transform the vertex and normal into world space.
  p = mul(float4(pos,1.0),gMatW);
  n = mul(float4(normal,1.0),gMatWN);
  n = normalize(n);
  
  // Compute diffuse component.
float3 l = normalize(gLight);

  o.diff = max(0.25,dot(n,l));
  
  // Compute a vector from the vertex to the eye.
float4  v = p - gEye;

  v = normalize(v);

  // Compute the reflection vector.
float3 ref = reflect(v,n);

  // Write the texture coordinates.
  o.tex0 = tex0;
  o.tex1 = normalize(ref);
  o.tex2 = tex0;
  o.view = v;
  
  // Finish vertex transformation.
  o.pos = mul(p,gMatVP);
  return (o);
}

/*  ________________________________________________________________________ */
VROUT VSRail(float3 pos    : POSITION,
             float3 normal : NORMAL)
/*! Vertex shader.
*/
{
VROUT   o;
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
  o.diff = max(0.25,dot(n,l));
  
  // Compute a vector from the vertex to the eye.
float4  v = gEye - p;

  v = normalize(v);

  // Compute the reflection vector.
float3 ref = -reflect(v,n);

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
float4  diffuse = p.diff * (0.80 * tex2D(sampTexture,p.tex0) + 0.20 * texCUBE(sampEnvirons,p.tex1));
float4  caustic = 0.25   * tex2D(sampCaustic,p.tex2);


  // Sample texture and write.
  o.color = diffuse + caustic;
  o.color.a = 1.0f;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSRail(VROUT p)
/*! Pixel shader.
*/
{
POUT    o;

  // Sample texture and write.
  o.color   = p.diff * texCUBE(sampEnvirons,p.refl);
  o.color.a = 1.0f;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSGhost(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;
float4  c;
float4  caustic = tex2D(sampCaustic,p.tex2);

  // Sample texture and write.
  o.color   = p.diff;
  o.color.a = 0.666;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSLite(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;
float4  diffuse = 2.0  * p.diff * (0.80 * tex2D(sampTexture,p.tex0) + 0.20 * texCUBE(sampEnvirons,p.tex1));
float4  caustic = 0.1  * tex2D(sampCaustic,p.tex2);

  // Sample texture and write.
  o.color = diffuse + caustic;
  o.color.a = 1.0f;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSDark(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;
float4  diffuse = 0.4 * p.diff * (0.80 * tex2D(sampTexture,p.tex0) + 0.20 * texCUBE(sampEnvirons,p.tex1));
float4  caustic = 0.1 * tex2D(sampCaustic,p.tex2);

  // Sample texture and write.
  o.color = diffuse + caustic;
  o.color.a = 1.0f;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSCue(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;
float4  diffuse = 2.0  * p.diff * (0.80 * tex2D(sampTexture,p.tex0) + 0.20 * texCUBE(sampEnvirons,p.tex1));
float4  caustic = 0.1  * tex2D(sampCaustic,p.tex2);

  // Sample texture and write.
  o.color = diffuse + caustic;
  o.color.a = 0.5f;
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSPart(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;

  o.color = float4(0.5,0.5,1.0,0.25);
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSBoom(VOUT p)
/*! Pixel shader.
*/
{
POUT    o;

  o.color = float4(1.0,0.0,1.0,0.50);
  return (o);
}


/*                                                                techniques
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
technique BallV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSMain();
  }
}

/*  ________________________________________________________________________ */
technique BallRailV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSRail();
    PixelShader  = compile ps_1_1 PSRail();
  }
}

/*  ________________________________________________________________________ */
technique BallGhostV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSGhost();
  }
}

/*  ________________________________________________________________________ */
technique BallLiteV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSLite();
  }
}

/*  ________________________________________________________________________ */
technique BallDarkV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSDark();
  }
}

/*  ________________________________________________________________________ */
technique BallCueV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSCue();
  }
}

/*  ________________________________________________________________________ */
technique BallPartV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSPart();
  }
}

/*  ________________________________________________________________________ */
technique BallExploV11P11
/*! VS1.1 and PS1.1
*/
{
  pass Pass0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSBoom();
  }
}
