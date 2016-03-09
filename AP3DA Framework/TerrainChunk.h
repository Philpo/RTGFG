#pragma once
#include "GameObject.h"

class TerrainChunk : public GameObject {
public:
  TerrainChunk(Material material, int xOffset, int zOffset, int height, int width, int terrainWidth, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext);
  ~TerrainChunk();
  inline const UINT* const getIndices() { return indices[0]; }
  inline ID3D11Buffer* const getIndexBuffer() { return indexBuffer; }
  void setCameraPosition(XMFLOAT3 cameraPosition);
  void setCentre(XMFLOAT3 centre) { this->centre = centre; }
  void Update(float t) override;
  void Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) override;
private:
  int* numberOfIndices;
  float* mipMapDistances;
  int numMipLevels, currentMipLevel;
  bool cameraMoved;
  XMFLOAT3 cameraPosition, centre;
  UINT** indices;
  ID3D11Buffer* indexBuffer;
  ID3D11DeviceContext* immediateContext;
};