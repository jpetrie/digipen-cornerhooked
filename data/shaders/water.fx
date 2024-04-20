/* =========================================================================
   
    @file    water.fx
    @author  jmp
    @brief   Water shader.
    
   ========================================================================= */

/*                                                                 variables
---------------------------------------------------------------------------- */

uniform float   gTime;      // Wave simulation time.

uniform float4  gLight;     // Vector from light source, world space.
uniform float4  gAmbient;   // Ambient (sun) color. RGBA.

uniform float4  gWAmp;      //!< Wave amplitudes.
uniform float4  gWFrq;      //!< Wave frequencies.
uniform float4  gWSpd;      //!< Wave speeds.
uniform float4  gWVX;       //!< Wave vector X components.
uniform float4  gWVY;       //!< Wave vector Y components.

uniform float4  gEye;       //!< Position (world space) of the camera.

float4x4        gMatW;      //!< World transform matrix.
float4x4        gMatWN;     //!< inverse(transpose(gMatW)), for normal transforms.
float4x4        gMatVP;     //!< View and projection matrix.

textureCUBE     gEnvMap;    //!< Texture to use as environment map.
texture2D       gBumpMap;   //!< Texture containing bump map.
texture         gCosLUT;    //!< Texture containing cosine LUT.
texture         gNoiseLUT;  //!< Texture containing noise LUT.
                                                                                  
float4  gTrans0;            //!< Wave translation for wave 1.
float4  gTrans1;            //!< Wave translation for wave 2.
float4  gTrans2;            //!< Wave translation for wave 3.
float4  gTrans3;            //!< Wave translation for wave 4.
float4  gTrans4;            //!< Wave translation for wave 5.
float4  gTrans5;            //!< Wave translation for wave 6.
float4  gTrans6;            //!< Wave translation for wave 7.
float4  gTrans7;            //!< Wave translation for wave 8.
float4  gTrans8;            //!< Wave translation for wave 9.
float4  gTrans9;            //!< Wave translation for wave 10.
float4  gTrans10;           //!< Wave translation for wave 11.
float4  gTrans11;           //!< Wave translation for wave 12.
float4  gTrans12;           //!< Wave translation for wave 13.
float4  gTrans13;           //!< Wave translation for wave 14.
float4  gTrans14;           //!< Wave translation for wave 15.
float4  gTrans15;           //!< Wave translation for wave 16.
float4  gCoeff0;            //!< Wave coefficient for wave 1.
float4  gCoeff1;            //!< Wave coefficient for wave 2.
float4  gCoeff2;            //!< Wave coefficient for wave 3.
float4  gCoeff3;            //!< Wave coefficient for wave 4.
float4  gCoeff4;            //!< Wave coefficient for wave 5.
float4  gCoeff5;            //!< Wave coefficient for wave 6.
float4  gCoeff6;            //!< Wave coefficient for wave 7.
float4  gCoeff7;            //!< Wave coefficient for wave 8.
float4  gCoeff8;            //!< Wave coefficient for wave 9.
float4  gCoeff9;            //!< Wave coefficient for wave 10.
float4  gCoeff10;           //!< Wave coefficient for wave 11.
float4  gCoeff11;           //!< Wave coefficient for wave 12.
float4  gCoeff12;           //!< Wave coefficient for wave 13.
float4  gCoeff13;           //!< Wave coefficient for wave 14.
float4  gCoeff14;           //!< Wave coefficient for wave 15.
float4  gCoeff15;           //!< Wave coefficient for wave 16.
float4  gRescale;           //!< Wave rescaling factor to bias to color range.

float4  gNoiseBias0_00;     //!< Normal map noise vector 1.
float4  gNoiseBias0_10;     //!< Normal map noise vector 2.
float4  gNoiseBias1_00;     //!< Normal map noise vector 3.
float4  gNoiseBias1_10;     //!< Normal map noise vector 4.
float4  gScaleBias;         //!< Noise scale bias factors.

const float     kRefIdx = 1.00 / 1.33;  //!<  Index of refraction ratio (air to water).

const float     kPi       = 3.14159265;  //!<  Pi...
const float     kTwoPi    = 6.28318530;  //!<  ...two pi...
const float     kInvTwoPi = 0.15915494;  //!<  ...I think you get it.

const float4    kOneVec = float4(1.0,1.0,1.0,1.0);  //!<  For vector component sums.

// The following two constants are constant terms in the Taylor Series
// approximation to sine and cosine.
const float4    kTASinTerm  = float4(1.0,(-1.0 / 6.0),(1.0 / 120.0),(-1.0 / 5040.0));
const float4    kTACosTerms = float4(1.0,(-1.0 / 2.0),(1.0 / 24.0),(-1.0 / 720.0));


/*                                                                  samplers
---------------------------------------------------------------------------- */

samplerCUBE sampEnv = sampler_state
{
  Texture   = <gEnvMap>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};

sampler2D sampBump = sampler_state
{
  Texture   = <gBumpMap>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};

sampler sampCos = sampler_state
{
  Texture   = <gCosLUT>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};

sampler sampNoise = sampler_state
{
  Texture   = <gNoiseLUT>;
  MinFilter = LINEAR;
  MagFilter = LINEAR;
  MipFilter = LINEAR;
};


/*                                                                   structs
---------------------------------------------------------------------------- */

struct VOUT
//! Represents output from water vertex shader.
{
  float4  pos  : POSITION;   //!< Final vertex position.
  float3  rfl  : TEXCOORD0;  //!< Reflection vector.
  float3  rfr  : TEXCOORD1;  //!< Refraction vector.
  float2  texc : TEXCOORD2;  //!< Bump map texture coordinates.
};

struct VNOUT
//! Represents output from water normal vertex shader.
{
  float4  pos  : POSITION;   //!< Final vertex position.
  float4  tex0 : TEXCOORD0;  //!< Ripple 0 texture coords.
  float4  tex1 : TEXCOORD1;  //!< Ripple 1 texture coords.
  float4  tex2 : TEXCOORD2;  //!< Ripple 2 texture coords.
  float4  tex3 : TEXCOORD3;  //!< Ripple 3 texture coords.
};

struct POUT
//! Represents output from water pixel shader.
{
	float4  color : COLOR;  //!< Final pixel color.
};


/*                                                                   shaders
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
VOUT VSMain(float3 pos      : POSITION,
            float3 normal   : NORMAL,
            float2 texc     : TEXCOORD)
/*! Vertex shader.
*/
{
VOUT    o;  // Output.
float4  p;  // Vertex position (world space).
float3  n;  // Vertex normal (world space).

  // Generate texture coordinates based on grid position of vertex.
  

  // Our wave equation is Asin(f(v*p) + ts). First we will compute
  // the value of f(v*p) + ts (the interior of the sine). We begin
  // by computing the dot product of the grid position and wave
  // directions.
float4  wi = gWVX * pos.xxxx + gWVY * pos.yyyy;

  // Then scale by the frequencies.
  wi = wi * gWFrq;
  
  // And add in the time term.
  wi = wi + (gTime * gWSpd);
  
  // Inputs to Taylor series approximation must be in the range
  // [-pi..pi] or strange things result.
	wi = wi + kPi;
	wi = wi * kInvTwoPi;
	wi = frac(wi);
	wi = wi * kTwoPi;
	wi = wi - kPi;
  
  // Compute sines as a Taylor series (using the sin intrinsic costs us way
  // too many instruction slots). First take the inputs up to the 7th power.
float4  wi2 = wi  * wi;
float4  wi3 = wi2 * wi;
float4  wi4 = wi2 * wi2;
float4  wi5 = wi3 * wi2;
float4  wi6 = wi3 * wi3;
float4  wi7 = wi4 * wi3;

  // Perform approximation.
float4  ws = wi + wi3 * kTASinTerm.yyyy + wi5 * kTASinTerm.zzzz + wi7 * kTASinTerm.wwww;
float4  wc = kTACosTerms.xxxx + wi2 * kTACosTerms.yyyy + wi4 * kTACosTerms.zzzz + wi6 * kTACosTerms.wwww;
  
  // Scale the sines by the amplitudes to get the final wave heights,
  // then sum the heights and apply to the vertex.
float4  wh = gWAmp * ws;

  pos = pos + normal * dot(wh,kOneVec);
  
  // Compute partial derivatives in X and Y. These are
  // Afncos(f(v*p) + ts) where n is v.x for X and v.y for Y.
  // We have already computed the cosines term, so we simply
  // scale in the leading coeffecient.
float4  px = gWAmp * gWFrq * gWVX * wc;
float4  py = gWAmp * gWFrq * gWVY * wc;
  
  // Sum the partial terms together to get the X and Y components
  // of the new normal (Z component remains whatever it was in
  // model space).
float  nx = dot(px,kOneVec);
float  ny = dot(py,kOneVec);

  normal = float3(-nx + normal.x,-ny + normal.y,normal.z);
  normal = normalize(normal);

  // Transform the vertex and normal into world space.
  p = mul(float4(pos,1.0),gMatW);
  n = mul(float4(normal,1.0),gMatWN);
  n = normalize(n);
    
float4  e = normalize(gEye - p);  // Eye vector (from vertex to eye).
  
  // Compute the reflection vector.
  o.rfr = refract(-e,n,kRefIdx);
  o.rfl = reflect(-e,n);
  
  // Pass texture coordinates to the pixel shader
  // for performing bumpmap lookup.
  o.texc = texc;
  
  // Finish vertex transformation.
  o.pos = mul(p,gMatVP);
  return (o);
}

/*  ________________________________________________________________________ */
POUT PSMain(VOUT p)
/*! Pixel shader.
*/
{
POUT  o;
float4  normal = tex2D(sampBump,p.texc);

  normal = (normal * 2.0) - kOneVec;

  o.color    = texCUBE(sampEnv,p.rfl);// + dot(normal,gLight);
  o.color.b += 0.25;
  o.color.a  = 0.33;
  return (o);
}

/*  ________________________________________________________________________ */
VNOUT VSNrml(float4          pos    : POSITION,
             float4          texc   : TEXCOORD0,
             uniform float4  dir0, 
             uniform float4  dir1,  
             uniform float4  dir2,  
             uniform float4  dir3)   
/*! Vertex shader for water normal map.
*/
{
VNOUT   o;
float4  p;

  // Transform the vertex.
  p = mul(pos,gMatW);
  p = mul(p,gMatVP);
  o.pos = p;

  // Compute the inner cosine terms, store them in the 
  // U component of texture coordinates. The dirN uniforms
  // are the wave directions for the four waves we are
  // processing in this pass.
float4  uv = float4(0.0,0.0,0.0,1.0);

	uv.x   = dot(texc,dir0);
	o.tex0 = uv;
	uv.x   = dot(texc,dir1);
	o.tex1 = uv;
	uv.x   = dot(texc,dir2);
	o.tex2 = uv;
	uv.x   = dot(texc,dir3);
	o.tex3 = uv;
	return (o);
}


/*                                                                 techniques
---------------------------------------------------------------------------- */

/*  ________________________________________________________________________ */
technique WaterV11P11
/*! VS1.1 and PS1.1
*/
{
  pass P0
  {
    VertexShader = compile vs_1_1 VSMain();
    PixelShader  = compile ps_1_1 PSMain();
  }
}

/*  ________________________________________________________________________ */
technique WaterNormalV11P11
/*! VS1.1 and PS1.1; renders normal map.
    The pixel shaders are written in assembly since they compile to too many
    instructions in HLSL.
*/
{
  pass P0
  {
    // First pass; first four waves.
    VertexShader = compile vs_1_1 VSNrml(gTrans0,gTrans1,gTrans2,gTrans3);
    PixelShader  =
    asm
    {
      ps_1_1

      // Sample textures (computes the cosine of the inner term
      // calculated 
      tex  t0;  // tex(sampCos,p.tex0)
      tex  t1;  // tex(sampCos,p.tex1)
      tex  t2;  // tex(sampCos,p.tex2)
      tex  t3;  // tex(sampCos,p.tex3)

      // Composite cosines. Constant registers c0-c3 contain the
      // coeff's of the wave equation's cosine part.
      mul		r0,t0_bx2,c0;     // r0 <==  (t0 / 2.0) * coefficient 0.
      mad		r0,t1_bx2,c1,r0;  // r0 <+=  (t1 / 2.0) * coefficient 1.
      mad		r0,t2_bx2,c2,r0;  // r0 <+=  (t2 / 2.0) * coefficient 2.
      mad		r0,t3_bx2,c3,r0;  // r0 <+=  (t3 / 2.0) * coefficient 3.
      
      // r0 now contains the composited waves.
      // Scale and bias into color range for output.
      mul		r0,r0,c4;  // c4 contains the rescaling
      add		r0,r0,c4;  // vector (0.5,0.5,0.5,1.0).
    };

    // Shader constants for this pass.
    PixelShaderConstant1[0] = <gCoeff0>;
    PixelShaderConstant1[1] = <gCoeff1>;
    PixelShaderConstant1[2] = <gCoeff2>;
    PixelShaderConstant1[3] = <gCoeff3>;
    PixelShaderConstant1[4] = <gRescale>;

    // Samplers for this pass.
    Sampler[0] = (sampCos);
    Sampler[1] = (sampCos);
    Sampler[2] = (sampCos);
    Sampler[3] = (sampCos);

    // Blend options for this pass.
    SrcBlend   = One;
    DestBlend  = Zero;
  }

  pass P1
  {
    // Second pass; next four waves.
    VertexShader = compile vs_1_1 VSNrml(gTrans4,gTrans5,gTrans6,gTrans7);
    PixelShader  =
    asm
    {
      ps_1_1

      // Sample textures (computes the cosine of the inner term
      // calculated 
      tex  t0;  // tex(sampCos,p.tex0)
      tex  t1;  // tex(sampCos,p.tex1)
      tex  t2;  // tex(sampCos,p.tex2)
      tex  t3;  // tex(sampCos,p.tex3)

      // Composite cosines. Constant registers c0-c3 contain the
      // coeff's of the wave equation's cosine part.
      mul		r0,t0_bx2,c0;     // r0 <==  (t0 / 2.0) * coefficient 0.
      mad		r0,t1_bx2,c1,r0;  // r0 <+=  (t1 / 2.0) * coefficient 1.
      mad		r0,t2_bx2,c2,r0;  // r0 <+=  (t2 / 2.0) * coefficient 2.
      mad		r0,t3_bx2,c3,r0;  // r0 <+=  (t3 / 2.0) * coefficient 3.
      
      // r0 now contains the composited waves.
      // Scale and bias into color range for output.
      mul		r0,r0,c4;  // c4 contains the rescaling
      add		r0,r0,c4;  // vector (0.5,0.5,0.5,1.0).
    };
    
    // Shader constants for this pass.
    PixelShaderConstant1[0] = <gCoeff4>;
    PixelShaderConstant1[1] = <gCoeff5>;
    PixelShaderConstant1[2] = <gCoeff6>;
    PixelShaderConstant1[3] = <gCoeff7>;
    PixelShaderConstant1[4] = <gRescale>;

    // Samplers for this pass.
    Sampler[0] = (sampCos);
    Sampler[1] = (sampCos);
    Sampler[2] = (sampCos);
    Sampler[3] = (sampCos);

    // Blend options for this pass.
    SrcBlend  = One;
    DestBlend = One;
  }
  
  pass P2
  {
    // Third pass; next four waves.
    VertexShader = compile vs_1_1 VSNrml(gTrans8,gTrans9,gTrans10,gTrans11);
    PixelShader  =
    asm
    {
      ps_1_1

      // Sample textures (computes the cosine of the inner term
      // calculated 
      tex  t0;  // tex(sampCos,p.tex0)
      tex  t1;  // tex(sampCos,p.tex1)
      tex  t2;  // tex(sampCos,p.tex2)
      tex  t3;  // tex(sampCos,p.tex3)

      // Composite cosines. Constant registers c0-c3 contain the
      // coeff's of the wave equation's cosine part.
      mul		r0,t0_bx2,c0;     // r0 <==  (t0 / 2.0) * coefficient 0.
      mad		r0,t1_bx2,c1,r0;  // r0 <+=  (t1 / 2.0) * coefficient 1.
      mad		r0,t2_bx2,c2,r0;  // r0 <+=  (t2 / 2.0) * coefficient 2.
      mad		r0,t3_bx2,c3,r0;  // r0 <+=  (t3 / 2.0) * coefficient 3.
      
      // r0 now contains the composited waves.
      // Scale and bias into color range for output.
      mul		r0,r0,c4;  // c4 contains the rescaling
      add		r0,r0,c4;  // vector (0.5,0.5,0.5,1.0).
    };
    
    // Shader constants for this pass.
    PixelShaderConstant1[0] = <gCoeff8>;
    PixelShaderConstant1[1] = <gCoeff9>;
    PixelShaderConstant1[2] = <gCoeff10>;
    PixelShaderConstant1[3] = <gCoeff11>;
    PixelShaderConstant1[4] = <gRescale>;

    // Samplers for this pass.
    Sampler[0] = (sampCos);
    Sampler[1] = (sampCos);
    Sampler[2] = (sampCos);
    Sampler[3] = (sampCos);

    // Blend options for this pass.
    SrcBlend  = One;
    DestBlend = One;
  }
    
  pass P3
  {
    // Fourth pass; next four waves.
    VertexShader = compile vs_1_1 VSNrml(gTrans12,gTrans13,gTrans14,gTrans15);
    PixelShader  =
    asm
    {
      ps_1_1

      // Sample textures (computes the cosine of the inner term
      // calculated 
      tex  t0;  // tex(sampCos,p.tex0)
      tex  t1;  // tex(sampCos,p.tex1)
      tex  t2;  // tex(sampCos,p.tex2)
      tex  t3;  // tex(sampCos,p.tex3)

      // Composite cosines. Constant registers c0-c3 contain the
      // coeff's of the wave equation's cosine part.
      mul		r0,t0_bx2,c0;     // r0 <==  (t0 / 2.0) * coefficient 0.
      mad		r0,t1_bx2,c1,r0;  // r0 <+=  (t1 / 2.0) * coefficient 1.
      mad		r0,t2_bx2,c2,r0;  // r0 <+=  (t2 / 2.0) * coefficient 2.
      mad		r0,t3_bx2,c3,r0;  // r0 <+=  (t3 / 2.0) * coefficient 3.
      
      // r0 now contains the composited waves.
      // Scale and bias into color range for output.
      mul		r0,r0,c4;  // c4 contains the rescaling
      add		r0,r0,c4;  // vector (0.5,0.5,0.5,1.0).
    };
    
    // Shader constants for this pass.
    PixelShaderConstant1[0] = <gCoeff12>;
    PixelShaderConstant1[1] = <gCoeff13>;
    PixelShaderConstant1[2] = <gCoeff14>;
    PixelShaderConstant1[3] = <gCoeff15>;
    PixelShaderConstant1[4] = <gRescale>;

    // Samplers for this pass.
    Sampler[0] = (sampCos);
    Sampler[1] = (sampCos);
    Sampler[2] = (sampCos);
    Sampler[3] = (sampCos);

    // Blend options for this pass.
    SrcBlend  = One;
    DestBlend = One;
  }

	pass P4
	{
	  // Final pass. Blend with noise texture.
    Sampler[0] = (sampNoise);
    Sampler[1] = (sampNoise);

    VertexShader =
    asm
    {
      vs_1_1
      
      dcl_position  v0		
      dcl_texcoord  v7		

      // Write position.
			mov  oPos,v0;
			
			// Factor the noise values into the texture sampling.
			mov  r0.zw,c4.xxxz; 
			dp4  r0.x,v7,c0;
			dp4  r0.y,v7,c1;
			
			// Store "noisified" texture coordinates.
			mov  oT0,r0;
			
			// Repeat.
			dp4  r0.x,v7,c2;
			dp4  r0.y,v7,c3;
			mov  oT1,r0;
			
			// Pass scaling factors through in colors.
			mov  oD0,c5.xxzz;
			mov  oD1,c5.yyzz;
    };

    // Shader constants for this pass.
    VertexShaderConstant[0] = <gNoiseBias0_00>;
    VertexShaderConstant[1] = <gNoiseBias0_10>;
    VertexShaderConstant[2] = <gNoiseBias1_00>;
    VertexShaderConstant[3] = <gNoiseBias1_10>;
    VertexShaderConstant[4] = float4(0.0f,0.5f,1.0f,2.0f);
    VertexShaderConstant[5] = <gScaleBias>;

    // Blend vertex colors.
    PixelShader =
    asm
    {
      ps_1_1
      
      // Sample textures from noisy coordinates.
      tex  t0;
      tex  t1;
      
      // Modulate and add together vertex colors for final color.
      add  r0.rgb,t0_bias,t1_bias;
      add  r0.a,t0,t1;
      mad  r0.rgb,r0,v0,v1;
    };

    // Blend options for this pass.
    SrcBlend  = One;
    DestBlend = One;
	}
}
