#pragma once
#include "GameObject.h"

struct BoundingBox {
  XMFLOAT3 topLeft;
  int width, height, depth;
};

class TerrainChunk : public GameObject {
public:
  TerrainChunk(Material material, int xOffset, int zOffset, int height, int width, int terrainWidth, float nearPlane, float nearTopLeft, float tau, float verticalResolution, float* const heightMap, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext);
  ~TerrainChunk();
  inline const UINT* const getIndices() { return indices[0]; }
  inline ID3D11Buffer* const getIndexBuffer() { return indexBuffer; }

  BoundingBox getBoundingBox() const { return boundingBox; }

  void setVisible(bool visible) { this->visible = visible; }
  void setBoundingBox(XMFLOAT3 topLeft, float height);
  void setCameraPosition(XMFLOAT3 cameraPosition);
  void setCentre(XMFLOAT3 centre) { this->centre = centre; }
  void setNorthChunk(TerrainChunk* const north) { this->north = north; }
  void setSouthChunk(TerrainChunk* const south) { this->south = south; }
  void setEastChunk(TerrainChunk* const east) { this->east = east; }
  void setWestChunk(TerrainChunk* const west) { this->west = west; }
  void setCameraMoved(bool cameraMoved) { this->cameraMoved = cameraMoved; }
  void calcMipMapLevel();
  void update(float t) override;
  void draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) override;
private:
  int* numberOfIndices;
  float *mipMapDistances, *heightMap;
  int xOffset, zOffset, height, width, terrainWidth, numMipLevels, currentMipLevel;
  float cSquared, kC;
  bool cameraMoved, visible;
  bool refresh = false;
  XMFLOAT3 cameraPosition, centre;
  UINT** indices;
  ID3D11Buffer* indexBuffer;
  ID3D11DeviceContext* immediateContext;
  BoundingBox boundingBox;
  TerrainChunk *north, *south, *east, *west;

  void refreshIndexBuffer();
  void calcDn2();
  float getHeight(int i, int j);
};