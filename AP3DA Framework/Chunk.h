#pragma once
#include "GameObject.h"
#include "Voxel.h"
#include <vector>

class Chunk : public GameObject {
public:
  Chunk(const int height, const int width, const int depth, Geometry voxelGeometry, Material voxelMaterial, ID3D11Device* d3dDevice, ID3D11DeviceContext* pImmediateContext);
  ~Chunk();

  void Update(float t) override;
  void Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) override;
private:
  int chunkHeight, chunkWidth, chunkDepth;
  Voxel**** voxels;
  bool*** occlusionMap;
  ID3D11Buffer* instanceBuffer; 
  std::vector<InstanceData> instanceData;
  ID3D11DeviceContext* pImmediateContext;
};