#pragma once
#include "GameObject.h"
#include <random>
#include <string>
#include <vector>
#include <fstream>
#include <noise\noise.h>
#include <iostream>

struct SimpleVertex;

class Terrain : public GameObject {
public:
  Terrain(Material material);
  ~Terrain();

  inline int getNumVertices() { return numVertices; }
  inline int getNumIndices() { return numIndices; }
  inline const SimpleVertex* const getVertices() { return vertices; }
  inline const UINT* const getIndices() { return indices; }
  float getCameraHeight(float cameraPosX, float cameraPosZ) const;

  void loadHeightMap(int height, int width, std::string heightMapFilename);
  void diamondSquare(int size, int seedValue, float heightScale, float h);
  void circleHill(int height, int width, int seedValue, int iterations, int radiusMin, int radiusMax);
  void perlinNoise(int height, int width, double lowerXBound, double upperXBound, double lowerZBound, double upperZBound);
  void generateGeometry(int height, int width, float cellWidth = 1.0f, float cellDepth = 1.0f);
  void cleanupTerrain();
private:
  int numVertices, numIndices;
  float terrainHeight, terrainWidth, dX, dZ;
  SimpleVertex* vertices;
  UINT* indices;
  float* heightMap;
  float** heightMap2;

  void generateVertices();
  void generateIndices();
  void generateNormals();
  float getHeight(int i, int j);
  float getHeight2(int i, int j);
};