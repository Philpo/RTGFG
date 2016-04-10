#pragma once
#include <directxmath.h>

using namespace DirectX;

const int DEFERRED_BUFFERS = 2;

struct SimpleVertex {
  XMFLOAT3 posL;
  XMFLOAT3 normL;
  XMFLOAT2 tex;
};

struct InstanceData {
  XMFLOAT4X4 world;
};

struct SurfaceInfo {
  XMFLOAT4 ambientMtrl;
  XMFLOAT4 diffuseMtrl;
  XMFLOAT4 specularMtrl;
};

struct Light {
  XMFLOAT4 ambientLight;
  XMFLOAT4 diffuseLight;
  XMFLOAT4 specularLight;

  float SpecularPower;
  XMFLOAT3 lightVecW;
};

struct ConstantBuffer {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX projection;
  XMMATRIX shadowTransform;

  SurfaceInfo surface;
  Light light;

  XMFLOAT3 eyePosW;
  float hasTexture;
  float maxTessDistance;
  float minTessDistance;
  float minTessFactor;
  float maxTessFactor;
};