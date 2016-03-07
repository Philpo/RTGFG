#pragma once
#include <directxmath.h>

using namespace DirectX;

const int DEFERRED_BUFFERS = 2;

struct SimpleVertex {
  XMFLOAT3 PosL;
  XMFLOAT3 NormL;
  XMFLOAT2 Tex;
};

struct InstanceData {
  XMFLOAT4X4 world;
};

struct SurfaceInfo {
  XMFLOAT4 AmbientMtrl;
  XMFLOAT4 DiffuseMtrl;
  XMFLOAT4 SpecularMtrl;
};

struct Light {
  XMFLOAT4 AmbientLight;
  XMFLOAT4 DiffuseLight;
  XMFLOAT4 SpecularLight;

  float SpecularPower;
  XMFLOAT3 LightVecW;
};

struct ConstantBuffer {
  XMMATRIX World;
  XMMATRIX View;
  XMMATRIX Projection;
  XMMATRIX shadowTransform;

  SurfaceInfo surface;
  Light light;

  XMFLOAT3 EyePosW;
  float HasTexture;
  float gMaxTessDistance;
  float gMinTessDistance;
  float gMinTessFactor;
  float gMaxTessFactor;
};