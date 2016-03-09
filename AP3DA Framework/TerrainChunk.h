#pragma once
#include "GameObject.h"

struct BoundingBox {
  XMFLOAT3 topLeft;
  int width, height, depth;
};

class TerrainChunk : public GameObject {
public:
  TerrainChunk(Material material, int xOffset, int zOffset, int height, int width, int terrainWidth, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext);
  ~TerrainChunk();
  inline const UINT* const getIndices() { return indices[0]; }
  inline ID3D11Buffer* const getIndexBuffer() { return indexBuffer; }

  BoundingBox getBoundingBox() const { return boundingBox; }

  void setVisible(bool visible) { this->visible = visible; }
  void setBoundingBox(XMFLOAT3 topLeft, float height);
  void setCameraPosition(XMFLOAT3 cameraPosition);
  void setCentre(XMFLOAT3 centre) { this->centre = centre; }
  void Update(float t) override;
  void Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) override;
private:
  int* numberOfIndices;
  float* mipMapDistances;
  int height, width, numMipLevels, currentMipLevel;
  bool cameraMoved, visible;
  XMFLOAT3 cameraPosition, centre;
  UINT** indices;
  ID3D11Buffer* indexBuffer;
  ID3D11DeviceContext* immediateContext;
  BoundingBox boundingBox;
};