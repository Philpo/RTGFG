#pragma once
#include "GameObject.h"
#include "TerrainChunk.h"
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <noise\noise.h>
#include "noiseutils.h"
#include <iostream>

struct SimpleVertex;

using namespace noise;

class Terrain : public GameObject {
public:
  Terrain(Material material);
  ~Terrain();

  inline int getNumVertices() { return numVertices; }
  inline int getNumIndices() { return CHUNK_HEIGHT * CHUNK_WIDTH * 6; }
  inline const SimpleVertex* const getVertices() { return vertices; }
  inline const UINT* const getIndices() { return indices; }
  float getCameraHeight(float cameraPosX, float cameraPosZ) const;

  void loadHeightMap(int height, int width, std::string heightMapFilename);
  void diamondSquare(int size, int seedValue, float heightScale, float h);
  void circleHill(int height, int width, int seedValue, int iterations, int radiusMin, int radiusMax);
  void perlinNoise(int height, int width, double lowerXBound, double upperXBound, double lowerZBound, double upperZBound);
  void generateGeometry(int height, int width, float nearPlane, float nearTopLeft, float tau, float verticalResolution, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext, float cellWidth = 1.0f, float cellDepth = 1.0f);

  void setCameraPosition(XMFLOAT3 cameraPosition);
  void cameraRotated();
  void frustumCull(XMFLOAT4* frustumPlanes);
  void update(float t) override;
  void draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) override;
private:
  int numVertices, numIndices;
  float terrainHeight, terrainWidth, dX, dZ;
  SimpleVertex* vertices;
  UINT* indices;
  float* heightMap;
  vector<TerrainChunk*> chunks;
  Material material;
  static const int CHUNK_HEIGHT = 64;
  static const int CHUNK_WIDTH = 64;

  void generateVertices();
  void generateIndices(float nearPlane, float nearTopLeft, float tau, float verticalResolution, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext);
  void generateNormals();
  void setChunkCentres();
  void setNeighbouringChunks();
  float getHeight(int i, int j);
};