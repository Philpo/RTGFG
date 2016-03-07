struct SurfaceInfo {
  float4 ambientMtrl;
  float4 diffuseMtrl;
  float4 specularMtrl;
};

struct Light {
  float4 ambientLight;
  float4 diffuseLight;
  float4 specularLight;

  float specularPower;
  float3 lightVecW;
};

cbuffer ConstantBuffer : register(b0) {
  matrix world;
  matrix view;
  matrix projection;
  matrix shadowTransform;

  SurfaceInfo surface;
  Light light;

  float3 eyePosW;
  float hasTexture;
  float maxTessDistance;
  float minTessDistance;
  float minTessFactor;
  float maxTessFactor;
}

struct VS_INPUT {
  float4 posL : POSITION;
  float3 normL : NORMAL;
  float2 tex : TEXCOORD0;
};

struct INSTANCE_VS_INPUT {
  float4 posL : POSITION;
  float3 normL : NORMAL;
  float2 tex : TEXCOORD0;
  float4x4 world : WORLD;
  uint instanceId : SV_InstanceID;
};

struct VS_OUTPUT {
  float4 posH : SV_POSITION;
  float3 normW : NORMAL;
  float3 posW : POSITION;
  float3 posL : POS_LOCAL;
  float2 tex : TEXCOORD0;
  float tessFactor : TESS;
  float4 shadowPos : TEXCOORD1;
};

struct PatchTess {
  float edgeTess[3] : SV_TessFactor;
  float insideTess : SV_InsideTessFactor;
};

struct HullOut {
  float3 posW     : POSITION;
  float3 posL : POS_LOCAL;
  float3 normalW  : NORMAL;
  float2 tex      : TEXCOORD;
  float4 shadowPos : SHADOWPOS;
};

struct DomainOut {
  float4 posH     : SV_POSITION;
  float3 posW     : POSITION;
  float3 posL : POS_LOCAL;
  float3 normW  : NORMAL;
  float2 tex      : TEXCOORD;
  float4 shadowPos : SHADOWPOS;
};

VS_OUTPUT VS(VS_INPUT input) {
  VS_OUTPUT output = (VS_OUTPUT) 0;

  float4 posW = mul(input.posL, world);
  output.posW = posW.xyz;

  output.posH = mul(posW, view);
  output.posH = mul(output.posH, projection);
  output.tex = input.tex;

  float3 normalW = mul(float4(input.normL, 0.0f), world).xyz;
  output.normW = normalize(normalW);
  output.posL = input.posL.xyz;
  output.shadowPos = mul(posW, shadowTransform);

  float d = distance(output.posW, eyePosW);

  // Normalized tessellation factor. 
  // The tessellation is 
  //   0 if d >= gMinTessDistance and
  //   1 if d <= gMaxTessDistance.  
  float tess = saturate((minTessDistance - d) / (minTessDistance - maxTessDistance));

  // Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
  output.tessFactor = minTessFactor + tess * (maxTessFactor - minTessFactor);

  return output;
}

VS_OUTPUT INSTANCE_VS(INSTANCE_VS_INPUT input) {
  VS_OUTPUT output = (VS_OUTPUT) 0;

  float4 posW = mul(input.posL, input.world);
  output.posW = posW.xyz;

  output.posH = mul(posW, view);
  output.posH = mul(output.posH, projection);
  output.tex = input.tex;

  output.shadowPos = mul(posW, shadowTransform);

  float3 normalW = mul(float4(input.normL, 0.0f), input.world).xyz;
  output.normW = normalize(normalW);
  output.posL = input.posL.xyz;

  float d = distance(output.posW, eyePosW);

  // Normalized tessellation factor. 
  // The tessellation is 
  //   0 if d >= gMinTessDistance and
  //   1 if d <= gMaxTessDistance.  
  float tess = saturate((minTessDistance - d) / (minTessDistance - maxTessDistance));

  // Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
  output.tessFactor = minTessFactor + tess * (maxTessFactor - minTessFactor);

  return output;
}

// Hull shaders
PatchTess PatchHS(InputPatch<VS_OUTPUT, 3> patch, uint patchID : SV_PrimitiveID) {
  PatchTess pt;

  // Average tess factors along edges, and pick an edge tess factor for 
  // the interior tessellation.  It is important to do the tess factor
  // calculation based on the edge properties so that edges shared by 
  // more than one triangle will have the same tessellation factor.  
  // Otherwise, gaps can appear.
  pt.edgeTess[0] = 0.5f*(patch[1].tessFactor + patch[2].tessFactor);
  pt.edgeTess[1] = 0.5f*(patch[2].tessFactor + patch[0].tessFactor);
  pt.edgeTess[2] = 0.5f*(patch[0].tessFactor + patch[1].tessFactor);
  pt.insideTess = pt.edgeTess[0];

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
  hout.posW = p[i].posW;
  hout.posL = p[i].posL;
  hout.normalW = p[i].normW;
  hout.tex = p[i].tex;
  hout.shadowPos = p[i].shadowPos;

  return hout;
}

// Domain Shader
[domain("tri")]
DomainOut DS(PatchTess patchTess, float3 bary : SV_DomainLocation, const OutputPatch<HullOut, 3> tri) {
  DomainOut dout;

  // Interpolate patch attributes to generated vertices.
  dout.posW = bary.x*tri[0].posW + bary.y*tri[1].posW + bary.z*tri[2].posW;
  dout.posL = bary.x*tri[0].posL + bary.y*tri[1].posL + bary.z*tri[2].posL;
  dout.normW = bary.x*tri[0].normalW + bary.y*tri[1].normalW + bary.z*tri[2].normalW;
  dout.tex = bary.x*tri[0].tex + bary.y*tri[1].tex + bary.z*tri[2].tex;
  dout.shadowPos = bary.x*tri[0].shadowPos + bary.y*tri[1].shadowPos + bary.z*tri[2].shadowPos;

  // Interpolating normal can unnormalize it, so normalize it.
  dout.normW = normalize(dout.normW);

  //
  // Displacement mapping.
  //

  // Choose the mipmap level based on distance to the eye; specifically, choose
  // the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
  const float mipInterval = 20.0f;
  float mipLevel = clamp((distance(dout.posW, eyePosW) - mipInterval) / mipInterval, 0.0f, 6.0f);

  // Sample height map (stored in alpha channel).
  //float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;

  // Offset vertex along normal.
  //dout.PosW += (gHeightScale*(h - 1.0))*dout.NormW;

  // Project to homogeneous clip space.
  dout.posH = mul(float4(dout.posW, 1.0f), view);
  dout.posH = mul(dout.posH, projection);

  return dout;
}