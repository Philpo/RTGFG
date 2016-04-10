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

SamplerState samLinear : register(s0);

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

struct VS_OUTPUT {
  float4 posH : SV_POSITION;
  float3 mormW : NORMAL;
  float3 posW : POSITION;
  float3 posL : POS_LOCAL;
  float2 tex : TEXCOORD0;
  float tessFactor : TESS;
  float4 shadowPos : TEXCOORD1;
};

struct DomainOut {
  float4 posH     : SV_POSITION;
  float3 posW     : POSITION;
  float3 posL : POS_LOCAL;
  float3 normW  : NORMAL;
  float2 tex      : TEXCOORD;
  float4 shadowPos : SHADOWPOS;
};

struct DeferredPixelOut {
  float3 colour : SV_TARGET0;
  float3 normal : SV_TARGET1;
};

DeferredPixelOut DEFERRED_PS(DomainOut input) : SV_Target {
  DeferredPixelOut pOut;

  pOut.colour = tex1.Sample(samLinear, input.tex).rgb;
  pOut.normal = normalize(input.normW);

  return pOut;
}

DeferredPixelOut DEFERRED_TERRAIN_PS(DomainOut input) : SV_Target {
  DeferredPixelOut pOut;

  pOut.normal = normalize(input.normW);

  // Get texture data from file
  float4 textureColour1 = tex1.Sample(samLinear, input.tex);
  float4 textureColour2 = tex2.Sample(samLinear, input.tex);
  float4 textureColour3 = tex3.Sample(samLinear, input.tex);
  float4 textureColour4 = tex4.Sample(samLinear, input.tex);
  float4 textureColour = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float lerpParam = 0.0f;

  if (abs(input.posL.y) >= 155.0f) {
    textureColour = textureColour1;
  }
  else if (abs(input.posL.y) < 155.0f && abs(input.posL.y) >= 105.0f) {
    lerpParam = abs(input.posL.y) - 105.0f;
    lerpParam /= 49.0f;
    textureColour = lerp(textureColour2, textureColour1, lerpParam);
  }
  else if (abs(input.posL.y) < 105.0f && abs(input.posL.y) >= 55.0f) {
    lerpParam = abs(input.posL.y) - 55.0f;
    lerpParam /= 49.0f;
    textureColour = lerp(textureColour3, textureColour2, lerpParam);
  }
  else if (abs(input.posL.y) < 55.0f) {
    lerpParam = abs(input.posL.y) / 55.0f;
    textureColour = lerp(textureColour4, textureColour3, lerpParam);
  }

  pOut.colour = textureColour.rgb;

  return pOut;
}

float4 LIGHTING_PS(VS_OUTPUT input) : SV_Target {
  float3 colour = tex1.Sample(samLinear, input.tex);
  float3 normalW = tex2.Sample(samLinear, input.tex);

  normalW = normalize(normalW);

  float3 ambient = float3(0.0f, 0.0f, 0.0f);
  float3 diffuse = float3(0.0f, 0.0f, 0.0f);
  float3 specular = float3(0.0f, 0.0f, 0.0f);

  float3 lightVecNorm = normalize(light.lightVecW);

  // Determine the diffuse light intensity that strikes the vertex.
  float diffuseAmount = max(dot(lightVecNorm, normalW), 0.0f);

  diffuse += diffuseAmount * (surface.diffuseMtrl * light.diffuseLight).rgb;
  ambient += (surface.ambientMtrl * light.ambientLight).rgb;

  // Sum all the terms together and copy over the diffuse alpha.
  float4 finalColour;

  if (colour.r == 0.0f && colour.g == 0.0f && colour.b == 0.0f) {
    colour = float4(1.0f, 1.0f, 1.0f, surface.diffuseMtrl.a);
  }

  finalColour.rgb = (colour * (ambient + diffuse)) + specular;

  finalColour.a = surface.diffuseMtrl.a;

  return finalColour;
}