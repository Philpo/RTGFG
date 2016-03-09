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

Texture2D tex1 : register(t0);
Texture2D tex2 : register(t1);
Texture2D tex3 : register(t2);
Texture2D tex4 : register(t3);
Texture2D shadowMap : register(t4);

SamplerState samLinear : register(s0);
SamplerComparisonState shadowSampler : register(s1);

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

struct DomainOut {
  float4 posH     : SV_POSITION;
  float3 posW     : POSITION;
  float3 posL : POS_LOCAL;
  float3 normW  : NORMAL;
  float2 tex      : TEXCOORD;
  float4 shadowPos : SHADOWPOS;
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

float calcShadowFactor(float4 shadowPos) {
  float depth = shadowPos.z - 0.002f;

  float dx = 1.0f / 2048.0f;

  float percentLit = 0.0f;

  float2 offsets[9] = {
    float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
    float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
    float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
  };

  for (int i = 0; i < 9; ++i) {
    percentLit += shadowMap.SampleCmpLevelZero(shadowSampler, shadowPos.xy + offsets[i], depth).r;
  }

  return percentLit /= 9.0f;
}

float4 SHADOW_PS(VS_OUTPUT input) : SV_Target {
  float depth = input.shadowPos.z - 0.001f;
  float3 shadowColour = float3(1.0f, 1.0f, 1.0f);
  //shadowColour = shadowMap.Sample(samLinear, input.shadowPos.xy).rgb;
  shadowColour.r = calcShadowFactor(input.shadowPos);
  //return float4(depth, depth, depth, 1.0f);
  //return float4(shadowColour.r, shadowColour.r, shadowColour.r, 1.0f);

  float3 normalW = normalize(input.normW);

  float3 toEye = normalize(eyePosW - input.posW);

  // Get texture data from file
  float4 textureColour = tex1.Sample(samLinear, input.tex);

  float3 ambient = float3(0.0f, 0.0f, 0.0f);
  float3 diffuse = float3(0.0f, 0.0f, 0.0f);
  float3 specular = float3(0.0f, 0.0f, 0.0f);

  float3 lightLecNorm = normalize(light.lightVecW);
  // Compute Colour

  // Compute the reflection vector.
  float3 r = reflect(-lightLecNorm, normalW);

  // Determine how much specular light makes it into the eye.
  float specularAmount = pow(max(dot(r, toEye), 0.0f), light.specularPower);

  // Determine the diffuse light intensity that strikes the vertex.
  float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);

  // Only display specular when there is diffuse
  if (diffuseAmount <= 0.0f) {
    specularAmount = 0.0f;
  }

  // Compute the ambient, diffuse, and specular terms separately.
  //if (depth <= shadowColour.r) {
  //  diffuse += (diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb);
  //  specular += (specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb);
  //}
  //else {
  //  diffuse += shadowColour.r * (diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb);
  //  specular += shadowColour.r * (specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb);
  //}
  diffuse += shadowColour.r * (diffuseAmount * (surface.diffuseMtrl * light.diffuseLight).rgb);
  specular += shadowColour.r * (specularAmount * (surface.specularMtrl * light.specularLight).rgb);
  //specular += (specularAmount * (surface.SpecularMtrl * light.SpecularLight).rgb);
  //diffuse += (diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb);
  ambient += (surface.ambientMtrl * light.ambientLight).rgb;

  // Sum all the terms together and copy over the diffuse alpha.
  float4 finalColour;

  if (hasTexture == 1.0f) {
    finalColour.rgb = (textureColour.rgb * (ambient + diffuse)) + specular;
  }
  else {
    finalColour.rgb = ambient + diffuse + specular;
  }

  finalColour.a = surface.diffuseMtrl.a;

  return finalColour;
}

float4 SHADOW_TERRAIN_PS(VS_OUTPUT input) : SV_Target{
  float depth = input.shadowPos.z - 0.001f;
  float3 shadowColour = float3(1.0f, 1.0f, 1.0f);
  //shadowColour = shadowMap.Sample(samLinear, input.shadowPos.xy).rgb;
  shadowColour.r = calcShadowFactor(input.shadowPos);
  //return float4(depth, depth, depth, 1.0f);
  //return float4(shadowColour.r, shadowColour.r, shadowColour.r, 1.0f);

  float3 normalW = normalize(input.normW);

  // Get texture data from file
  float4 textureColour1 = tex1.Sample(samLinear, input.tex);
  float4 textureColour2 = tex2.Sample(samLinear, input.tex);
  float4 textureColour3 = tex3.Sample(samLinear, input.tex);
  float4 textureColour4 = tex4.Sample(samLinear, input.tex);
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

  float3 ambient = float3(0.0f, 0.0f, 0.0f);
  float3 diffuse = float3(0.0f, 0.0f, 0.0f);

  float3 lightLecNorm = normalize(light.lightVecW);
  // Compute Colour

  // Determine the diffuse light intensity that strikes the vertex.
  float diffuseAmount = max(dot(lightLecNorm, normalW), 0.0f);

  // Compute the ambient, diffuse, and specular terms separately.
  //if (depth <= shadowColour.r) {
  //  diffuse += (diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb);
  //}
  //else {
  //  diffuse += shadowColour.r * (diffuseAmount * (surface.DiffuseMtrl * light.DiffuseLight).rgb);
  //}
  diffuse += shadowColour.r * (diffuseAmount * (surface.diffuseMtrl * light.diffuseLight).rgb);
  ambient += (surface.ambientMtrl * light.ambientLight).rgb;

  // Sum all the terms together and copy over the diffuse alpha.
  float4 finalColour;

  finalColour.rgb = (textureColour.rgb * (ambient + diffuse));
  finalColour.a = surface.diffuseMtrl.a;

  return finalColour;
}