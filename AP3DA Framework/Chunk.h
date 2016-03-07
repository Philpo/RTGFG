#pragma once
#include "GameObject.h"
#include "Voxel.h"
#include <vector>

class Chunk : public GameObject {
public:
  Chunk(const int height, const int width, const int depth, Geometry voxelGeometry, Material voxelMaterial, ID3D11Device* d3dDevice, ID3D11DeviceContext* pImmediateContext);
  ~Chunk();

  void perlinNoise2D(int seed, int width, int height, int octaves, float persistence, int zoom);
  void perlinNoise3D(int seed, int width, int height, int depth, int octaves, float persistence, int zoom);

  void Update(float t) override;
  void Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) override;
private:
  int chunkHeight, chunkWidth, chunkDepth, visibleVoxels;
  Voxel**** voxels;
  bool*** occlusionMap;
  float** heightMap2D;
  float*** heightMap3D;
  ID3D11Buffer* instanceBuffer; 
  std::vector<InstanceData> instanceData;
  ID3D11DeviceContext* pImmediateContext;
  ID3D11Device* d3dDevice;

  float random(int seed, int x, int y, int z);
  float lerp(float a, float b, float x);
};