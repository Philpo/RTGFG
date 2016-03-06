//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//--------------------------------------------------------------------------------------

Texture2D txDiffuse : register(t0);
Texture2D terrainTex1 : register(t1);
Texture2D terrainTex2 : register(t2);
Texture2D terrainTex3 : register(t3);
Texture2D terrainTex4 : register(t4);
//Texture2D colour : register(t5);
//Texture2D normal : register(t6);
//Texture2D materialA : register(t7);
//Texture2D materialD : register(t8);
//Texture2D materialS : register(t9);

SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

struct SurfaceInfo {
  float4 AmbientMtrl;
  float4 DiffuseMtrl;
  float4 SpecularMtrl;
};

struct Light {
  float4 AmbientLight;
  float4 DiffuseLight;
  float4 SpecularLight;

  float SpecularPower;
  float3 LightVecW;
};

cbuffer ConstantBuffer : register(b0) {
  matrix World;
  matrix View;
  matrix Projection;

  SurfaceInfo surface;
  Light light;

  float3 EyePosW;
  float HasTexture;
  float gMaxTessDistance;
  float gMinTessDistance;
  float gMinTessFactor;
  float gMaxTessFactor;
}

struct VS_INPUT {
  float4 PosL : POSITION;
  float3 NormL : NORMAL;
  float2 Tex : TEXCOORD0;
};

struct INSTANCE_VS_INPUT {
  float4 PosL : POSITION;
  float3 NormL : NORMAL;
  float2 Tex : TEXCOORD0;
  float4x4 world : WORLD;
  uint instanceId : SV_InstanceID;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT {
  float4 PosH : SV_POSITION;
  float3 NormW : NORMAL;

  float3 PosW : POSITION;
  float3 posL : POS_LOCAL;
  float2 Tex : TEXCOORD0;
  float TessFactor : TESS;
};

struct PatchTess {
  float EdgeTess[3] : SV_TessFactor;
  float InsideTess : SV_InsideTessFactor;
};

struct HullOut {
  float3 PosW     : POSITION;
  float3 posL : POS_LOCAL;
  float3 NormalW  : NORMAL;
  float2 Tex      : TEXCOORD;
};

struct DomainOut {
  float4 PosH     : SV_POSITION;
  float3 PosW     : POSITION;
  float3 posL : POS_LOCAL;
  float3 NormW  : NORMAL;
  float2 Tex      : TEXCOORD;
};

struct DeferredPixelOut {
  float4 colour : SV_TARGET0;
  float4 normal : SV_TARGET1;
  //float4 materialA : SV_TARGET2;
  //float4 materialD : SV_TARGET3;
  //float4 materialS : SV_TARGET4;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT DEFERRED_VS(VS_INPUT input) {
  VS_OUTPUT output = (VS_OUTPUT) 0;

  float4 posW = mul(input.PosL, World);
  output.PosW = posW.xyz;

  output.PosH = mul(posW, View);
  output.PosH = mul(output.PosH, Projection);
  output.Tex = input.Tex;

  float3 normalW = mul(float4(input.NormL, 0.0f), World).xyz;
  output.NormW = normalize(normalW);
  output.posL = input.PosL.xyz;

  float d = distance(output.PosW, EyePosW);

  // Normalized tessellation factor. 
  // The tessellation is 
  //   0 if d >= gMinTessDistance and
  //   1 if d <= gMaxTessDistance.  
  float tess = saturate((gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance));

  // Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
  output.TessFactor = gMinTessFactor + tess*(gMaxTessFactor - gMinTessFactor);

  return output;
}

VS_OUTPUT INSTANCE_VS(INSTANCE_VS_INPUT input) {
  VS_OUTPUT output = (VS_OUTPUT) 0;

  float4 posW = mul(input.PosL, input.world);
  output.PosW = posW.xyz;

  output.PosH = mul(posW, View);
  output.PosH = mul(output.PosH, Projection);
  output.Tex = input.Tex;

  float3 normalW = mul(float4(input.NormL, 0.0f), input.world).xyz;
  output.NormW = normalize(normalW);
  output.posL = input.PosL.xyz;

  float d = distance(output.PosW, EyePosW);

  // Normalized tessellation factor. 
  // The tessellation is 
  //   0 if d >= gMinTessDistance and
  //   1 if d <= gMaxTessDistance.  
  float tess = saturate((gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance));

  // Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
  output.TessFactor = gMinTessFactor + tess * (gMaxTessFactor - gMinTessFactor);

  return output;
}

VS_OUTPUT VS(VS_INPUT input) {
  VS_OUTPUT vOut = (VS_OUTPUT) 0;

  float4 posW = mul(input.PosL, World);
  vOut.PosW = posW.xyz;

  vOut.PosH = mul(posW, View);
  vOut.PosH = mul(vOut.PosH, Projection);
  vOut.Tex = input.Tex;

  return vOut;
}

// Hull shaders
PatchTess PatchHS(InputPatch<VS_OUTPUT, 3> patch, uint patchID : SV_PrimitiveID) {
  PatchTess pt;

  // Average tess factors along edges, and pick an edge tess factor for 
  // the interior tessellation.  It is important to do the tess factor
  // calculation based on the edge properties so that edges shared by 
  // more than one triangle will have the same tessellation factor.  
  // Otherwise, gaps can appear.
  pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
  pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
  pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
  pt.InsideTess = pt.EdgeTess[0];

  return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<VS_OUTPUT, 3> p, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID) {
  HullOut hout;

  // Pass through shader.
  hout.PosW = p[i].PosW;
  hout.posL = p[i].posL;
  hout.NormalW = p[i].NormW;
  hout.Tex = p[i].Tex;

  return hout;
}

// Domain Shader
[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri) {
  DomainOut dout;

  // Interpolate patch attributes to generated vertices.
  dout.PosW = bary.x*tri[0].PosW + bary.y*tri[1].PosW + bary.z*tri[2].PosW;
  dout.posL = bary.x*tri[0].posL + bary.y*tri[1].posL + bary.z*tri[2].posL;
  dout.NormW = bary.x*tri[0].NormalW + bary.y*tri[1].NormalW + bary.z*tri[2].NormalW;
  dout.Tex = bary.x*tri[0].Tex + bary.y*tri[1].Tex + bary.z*tri[2].Tex;

  // Interpolating normal can unnormalize it, so normalize it.
  dout.NormW = normalize(dout.NormW);

  //
  // Displacement mapping.
  //

  // Choose the mipmap level based on distance to the eye; specifically, choose
  // the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
  const float MipInterval = 20.0f;
  float mipLevel = clamp((distance(dout.PosW, EyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);

  // Sample height map (stored in alpha channel).
  //float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;

  // Offset vertex along normal.
  //dout.PosW += (gHeightScale*(h - 1.0))*dout.NormW;

  // Project to homogeneous clip space.
  dout.PosH = mul(float4(dout.PosW, 1.0f), View);
  dout.PosH = mul(dout.PosH, Projection);

  return dout;
}


// deferred pixel shaders
DeferredPixelOut DEFERRED_PS(DomainOut input) : SV_Target
{
  DeferredPixelOut pOut;
  pOut.normal = float4(normalize(input.NormW), 0.0f);
  pOut.colour = txDiffuse.Sample(samLinear, input.Tex);
  //pOut.materialA = surface.AmbientMtrl;
  //pOut.materialD = surface.DiffuseMtrl;
  //pOut.materialS = surface.SpecularMtrl;

  return pOut;
}

DeferredPixelOut TERRAIN_PS(DomainOut input) : SV_Target
{
  DeferredPixelOut pOut;
  pOut.normal = float4(normalize(input.NormW), 0.0f);

  // Get texture data from file
  float4 textureColour1 = terrainTex1.Sample(samLinear, input.Tex);
  float4 textureColour2 = terrainTex2.Sample(samLinear, input.Tex);
  float4 textureColour3 = terrainTex3.Sample(samLinear, input.Tex);
  float4 textureColour4 = terrainTex4.Sample(samLinear, input.Tex);
  float4 textureColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float lerpParam = 0.0f;

  if (abs(input.posL.y) >= 205.0f) {
    textureColour = textureColour1;
  }
  else if (abs(input.posL.y) < 205.0f && abs(input.posL.y) >= 155.0f) {
    lerpParam = abs(input.posL.y) - 155.0f;
    lerpParam /= 49.0f;
    textureColour = lerp(textureColour2, textureColour1, lerpParam);
  }
  else if (abs(input.posL.y) < 155.0f && abs(input.posL.y) >= 105.0f) {
    lerpParam = abs(input.posL.y) - 105.0f;
    lerpParam /= 49.0f;
    textureColour = lerp(textureColour3, textureColour2, lerpParam);
  }
  else if (abs(input.posL.y) < 105.0f) {
    lerpParam = abs(input.posL.y) / 104.0f;
    textureColour = lerp(textureColour4, textureColour3, lerpParam);
  }

  pOut.colour = textureColour;
  //pOut.materialA = surface.AmbientMtrl;
  //pOut.materialD = surface.DiffuseMtrl;
  //pOut.materialS = surface.SpecularMtrl;

  return pOut;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
  float4 colour = txDiffuse.Sample(samLinear, input.Tex);
  float3 normalW = terrainTex1.Sample(samLinear, input.Tex).xyz;
  //float4 materialA = terrainTex2.Sample(samLinear, input.Tex);
  //float4 materialD = terrainTex3.Sample(samLinear, input.Tex);
  //float4 materialS = terrainTex4.Sample(samLinear, input.Tex);

  normalW = normalize(normalW);

  // TODO
  //float3 toEye = normalize(EyePosW - input.PosW);

  float3 ambient = float3(0.0f, 0.0f, 0.0f);
  float3 diffuse = float3(0.0f, 0.0f, 0.0f);
  float3 specular = float3(0.0f, 0.0f, 0.0f);

  float3 lightLecNorm = normalize(light.LightVecW);
  // Compute Colour

  // Compute the reflection vector.
  //float3 r = reflect(-lightLecNorm, normalW);

  // Determine how much specular light makes it into the eye.
  //float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);

  // Determine the diffuse light intensity that strikes the vertex.
  float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);

  // Only display specular when there is diffuse
  //if (diffuseAmount <= 0.0f) {
  //  specularAmount = 0.0f;
  //}

  // Compute the ambient, diffuse, and specular terms separately.
  //specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
  diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
  ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;

  // Sum all the terms together and copy over the diffuse alpha.
  float4 finalColour;

  if (colour.r == 0.0f && colour.g == 0.0f && colour.b == 0.0f) {
    colour = float4(1.0f, 1.0f, 1.0f, colour.a);
  }

  finalColour.rgb = (colour.rgb * (ambient + diffuse)) + specular;

  finalColour.a = surface.DiffuseMtrl.a;

  return finalColour;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
//float4 PS(DomainOut input) : SV_Target
//{
//  float3 normalW = normalize(input.NormW);
//
//  float3 toEye = normalize(EyePosW - input.PosW);
//
//  // Get texture data from file
//  float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);
//
//  float3 ambient = float3(0.0f, 0.0f, 0.0f);
//  float3 diffuse = float3(0.0f, 0.0f, 0.0f);
//  float3 specular = float3(0.0f, 0.0f, 0.0f);
//
//  float3 lightLecNorm = normalize(light.LightVecW);
//  // Compute Colour
//
//  // Compute the reflection vector.
//  float3 r = reflect(-lightLecNorm, normalW);
//
//  // Determine how much specular light makes it into the eye.
//  float specularAmount = pow(max(dot(r, toEye), 0.0f), light.SpecularPower);
//
//  // Determine the diffuse light intensity that strikes the vertex.
//  float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);
//
//  // Only display specular when there is diffuse
//  if (diffuseAmount <= 0.0f) {
//    specularAmount = 0.0f;
//  }
//
//  // Compute the ambient, diffuse, and specular terms separately.
//  specular += specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb;
//  diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
//  ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;
//
//  // Sum all the terms together and copy over the diffuse alpha.
//  float4 finalColour;
//
//  if (HasTexture == 1.0f) {
//    finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
//  }
//  else {
//    finalColour.rgb = ambient + diffuse + specular;
//  }
//
//  finalColour.a = surface.DiffuseMtrl.a;
//
//  return finalColour;
//}
//
//float4 TERRAIN_PS(DomainOut input) : SV_Target
//{
//  float3 normalW = normalize(input.NormW);
//
//  // Get texture data from file
//  float4 textureColour1 = terrainTex1.Sample(samLinear, input.Tex);
//  float4 textureColour2 = terrainTex2.Sample(samLinear, input.Tex);
//  float4 textureColour3 = terrainTex3.Sample(samLinear, input.Tex);
//  float4 textureColour4 = terrainTex4.Sample(samLinear, input.Tex);
//  float4 textureColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
//  float lerpParam = 0.0f;
//
//  if (abs(input.posL.y) >= 205.0f) {
//    textureColour = textureColour1;
//  }
//  else if (abs(input.posL.y) < 205.0f && abs(input.posL.y) >= 155.0f) {
//    lerpParam = abs(input.posL.y) - 155.0f;
//    lerpParam /= 49.0f;
//    textureColour = lerp(textureColour2, textureColour1, lerpParam);
//  }
//  else if (abs(input.posL.y) < 155.0f && abs(input.posL.y) >= 105.0f) {
//    lerpParam = abs(input.posL.y) - 105.0f;
//    lerpParam /= 49.0f;
//    textureColour = lerp(textureColour3, textureColour2, lerpParam);
//  }
//  else if (abs(input.posL.y) < 105.0f) {
//    lerpParam = abs(input.posL.y) / 104.0f;
//    textureColour = lerp(textureColour4, textureColour3, lerpParam);
//  }
//
//  float3 ambient = float3(0.0f, 0.0f, 0.0f);
//  float3 diffuse = float3(0.0f, 0.0f, 0.0f);
//
//  float3 lightLecNorm = normalize(light.LightVecW);
//  // Compute Colour
//
//  // Determine the diffuse light intensity that strikes the vertex.
//  float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);
//
//  // Compute the ambient, diffuse, and specular terms separately.
//  diffuse += diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb;
//  ambient += (surface.AmbientMtrl * light.AmbientLight).rgb;
//
//  // Sum all the terms together and copy over the diffuse alpha.
//  float4 finalColour;
//
//  finalColour.rgb = (textureColour.rgb * (ambient + diffuse));
//  finalColour.a = surface.DiffuseMtrl.a;
//
//  return finalColour;
//}