#include "Terrain.h"
#include "Application.h"
#include <iostream>
#include <fstream>

using namespace std;

float avgDiamondVals(int i, int j, int stride, int size, int subSize, float* fa);
float avgSquareVals(int i, int j, int stride, int size, float* fa);
float randFloat(float min, float max);
int randInt(int min, int max);
float parabola(int centreX, int centreZ, int pointX, int pointZ, int radius);

Terrain::Terrain(Material material) : GameObject("terrain", material), material(material), vertices(nullptr), indices(nullptr), heightMap(nullptr) {}

Terrain::~Terrain() {
  if (vertices) {
    delete[] vertices;
  }
  if (indices) {
    delete[] indices;
  }
  if (heightMap) {
    delete[] heightMap;
  }

  for (auto chunk : chunks) {
    delete chunk;
  }
}

void Terrain::setCameraPosition(XMFLOAT3 cameraPosition) {
  for (auto chunk : chunks) {
    chunk->setCameraPosition(cameraPosition);
  }
}

void Terrain::cameraRotated() {
  for (auto chunk : chunks) {
    chunk->setCameraMoved(true);
  }
}

void Terrain::frustumCull(XMFLOAT4* frustumPlanes) {
  XMVECTOR dotProduct;
  XMFLOAT3 temp;
  XMFLOAT4 distanceFromTop;
  XMFLOAT4 distanceFromBottom;
  XMFLOAT3 trimmedPlane;
  BoundingBox boundingBox;

  for (auto chunk : chunks) {
    for (int i = 0; i < 6; i++) {
      trimmedPlane = XMFLOAT3(frustumPlanes[i].x, frustumPlanes[i].y, frustumPlanes[i].z);

      boundingBox = chunk->getBoundingBox();
      // top top left corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedPlane)));
      distanceFromTop.x = temp.x + frustumPlanes[i].w;

      // top top right corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedPlane)));
      distanceFromTop.y = temp.x + frustumPlanes[i].w;

      // top bottom left corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedPlane)));
      distanceFromTop.z = temp.x + frustumPlanes[i].w;

      // top bottom right corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedPlane)));
      distanceFromTop.w = temp.x + frustumPlanes[i].w;

      // bottom top left corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedPlane)));
      distanceFromBottom.x = temp.x + frustumPlanes[i].w;

      // bottom top right corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedPlane)));
      distanceFromBottom.y = temp.x + frustumPlanes[i].w;

      // bottom bottom left corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedPlane)));
      distanceFromBottom.z = temp.x + frustumPlanes[i].w;

      // bottom bottom right corner
      XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedPlane)));
      distanceFromBottom.w = temp.x + frustumPlanes[i].w;

      if (distanceFromTop.x > 0 && distanceFromTop.y > 0 && distanceFromTop.z > 0 && distanceFromTop.w > 0 && distanceFromBottom.x > 0 && distanceFromBottom.y > 0 && distanceFromBottom.z > 0 && distanceFromBottom.w > 0) {
        chunk->setVisible(false);
        break;
      }

      chunk->setVisible(true);
    }
  }
}

void Terrain::update(float t) {
  for (auto chunk : chunks) {
    chunk->calcMipMapLevel();
    chunk->update(t);
  }
}

void Terrain::draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
  for (auto chunk : chunks) {
    chunk->draw(cb, constantBuffer, pImmediateContext);
  }
}

// taken from Frank Luna 3D Game Programming with DirectX 11
float Terrain::getCameraHeight(float cameraPosX, float cameraPosZ) const {
  if (heightMap) {
    float c = (cameraPosX + 0.5f * (terrainWidth + 1)) / dX;
    float d = (cameraPosZ - 0.5f * (terrainHeight + 1)) / -dZ;

    int row = floorf(d);
    int col = floorf(c);

    float A = heightMap[row * (((int) terrainWidth) + 1) + col];
    float B = heightMap[row * (((int) terrainWidth) + 1) + col + 1];
    float C = heightMap[(row + 1) * (((int) terrainWidth) + 1) + col];
    float D = heightMap[(row + 1) * (((int) terrainWidth) + 1) + col + 1];

    // Where we are relative to the cell.
    float s = c - (float) col;
    float t = d - (float) row;

    // If upper triangle ABC.
    if (s + t <= 1.0f) {
      float uy = B - A;
      float vy = C - A;
      return A + s * uy + t * vy;
    }
    // lower triangle DCB.
    else {
      float uy = C - D;
      float vy = B - D;
      return D + (1.0f - s) * uy + (1.0f - t) * vy;
    }
  }
  else {
    return 0.0f;
  }
}

void Terrain::loadHeightMap(int height, int width, std::string heightMapFilename) {
  // A height for each vertex 
  std::vector<unsigned char> in((height + 1) * (width + 1));
  heightMap = new float[(height + 1) * (width + 1)];

  // Open the file.
  std::ifstream inFile;
  inFile.open(heightMapFilename.c_str(), std::ios_base::binary);

  if (inFile) {
    // Read the RAW bytes.
    inFile.read((char*) &in[0], (std::streamsize)in.size());
    // Done with file.
    inFile.close();
  }

  // Copy the array data into a float array and scale it. heightMap.resize(heightmapHeight * heightmapWidth, 0);

  for (UINT i = 0; i < (height + 1) * (width + 1); ++i) {
    heightMap[i] = in[i];
  }
}

// adapted from http://gameprogrammer.com/fractal.html#structure
void Terrain::diamondSquare(int size, int seedValue, float heightScale, float h) {
  int	stride, firstLine, subSize;
  float ratio, scale;

  subSize = size;
  size++;
  heightMap = new float[size * size];
  for (int i = 0; i < size * size; i++) {
    heightMap[i] = 0.0f;
  }
  srand(seedValue);

  ratio = (float) pow(2.0, -h);
  scale = heightScale * ratio;

  stride = subSize / 2;

  while (stride) {
    for (int i = stride; i < subSize; i += stride) {
      for (int j = stride; j < subSize; j += stride) {
        heightMap[(i * size) + j] = randFloat(-h, h) + avgSquareVals(i, j, stride, size, heightMap);
        j += stride;
      }
      i += stride;
    }

    firstLine = 0;
    for (int i = 0; i < subSize; i += stride) {
      firstLine = (firstLine == 0);
      for (int j = 0; j < subSize; j += stride) {
        if (firstLine && !j) {
          j += stride;
        }

        heightMap[(i * size) + j] = randFloat(-h, h) + avgDiamondVals(i, j, stride, size, subSize, heightMap);

        if (i == 0) {
          heightMap[(subSize * size) + j] = heightMap[(i * size) + j];
        }
        if (j == 0) {
          heightMap[(i * size) + subSize] = heightMap[(i * size) + j];
        }

        j += stride;
      }
    }

    /* reduce random number range. */
    h /= 2;
    stride >>= 1;
  }
}

void Terrain::circleHill(int height, int width, int seedValue, int iterations, int radiusMin, int radiusMax) {
  int size = (height + 1) * (width + 1);
  heightMap = new float[size];
  for (int i = 0; i < size; i++) {
    heightMap[i] = 0.0f;
  }

  srand(seedValue);
  int centreX, centreZ, radius;

  for (int i = 0; i < iterations; i++) {
    cout << i;
    centreX = randInt(0, width + 1);
    centreZ = randInt(0, height + 1);
    radius = randInt(radiusMin, radiusMax);
    cout << " r = " << radius;
    cout << endl;

    for (int j = centreZ - radius; j <= centreZ + radius; j++) {
      if (j >= 0 && j < height + 1) {
        for (int k = centreX - radius; k <= centreX + radius; k++) {
          if (k >= 0 && k < width + 1) {
            float height = parabola(centreX, centreZ, k, j, radius);
            if (height > 0) {
              heightMap[(j * (width + 1)) + k] += height;
            }
          }
        }
      }
    }
  }

  float k = 0.5f;
  /* Rows, left to right */
  for (int x = 1; x < width + 1; x++) {
    for (int z = 0; z < height + 1; z++) {
      heightMap[(z * (width + 1)) + x] = heightMap[(z * (width + 1)) + x - 1] * (1 - k) + heightMap[(z * (width + 1)) + x] * k;
    }
  }

  /* Rows, right to left*/
  for (int x = width - 1; x < -1; x--) {
    for (int z = 0; z < height + 1; z++) {
      heightMap[(z * (width + 1)) + x] = heightMap[(z * (width + 1)) + x + 1] * (1 - k) + heightMap[(z * (width + 1)) + x] * k;
    }
  }

  /* Columns, bottom to top */
  for (int x = 0; x < width + 1; x++)
    for (int z = 1; z < height + 1; z++)
      heightMap[(z * (width + 1)) + x] = heightMap[((z - 1) * (width + 1)) + x] * (1 - k) + heightMap[(z * (width + 1)) + x] * k;

  /* Columns, top to bottom */
  for (int x = 0; x < width + 1; x++) {
    for (int z = height - 1; z < -1; z--) {
      heightMap[(z * (width + 1)) + x] = heightMap[((z + 1) * (width + 1)) + x] * (1 - k) + heightMap[(z * (width + 1)) + x] * k;
    }
  }
}

// taken from http://libnoise.sourceforge.net/tutorials/tutorial5.html
void Terrain::perlinNoise(int height, int width, double lowerXBound, double upperXBound, double lowerZBound, double upperZBound) {
  int size = (height + 1) * (width + 1);
  heightMap = new float[size];
  for (int i = 0; i < size; i++) {
    heightMap[i] = 0.0f;
  }

  module::RidgedMulti mountainTerrain;
  module::Billow baseFlatTerrain;
  baseFlatTerrain.SetFrequency(2.0);
  module::ScaleBias flatTerrain;
  flatTerrain.SetSourceModule(0, baseFlatTerrain);
  flatTerrain.SetScale(0.125);
  flatTerrain.SetBias(-0.75);
  module::Perlin terrainType;
  terrainType.SetFrequency(0.5);
  terrainType.SetPersistence(0.25);
  module::Select terrainSelector;
  terrainSelector.SetSourceModule(0, flatTerrain);
  terrainSelector.SetSourceModule(1, mountainTerrain);
  terrainSelector.SetControlModule(terrainType);
  terrainSelector.SetBounds(0.0, 10000.0);
  terrainSelector.SetEdgeFalloff(0.125);
  module::Turbulence finalTerrain;
  finalTerrain.SetSourceModule(0, terrainSelector);
  finalTerrain.SetFrequency(4.0);
  finalTerrain.SetPower(0.125);
  utils::NoiseMap noiseMap;
  utils::NoiseMapBuilderPlane heightMapBuilder;
  heightMapBuilder.SetSourceModule(finalTerrain);
  heightMapBuilder.SetDestNoiseMap(noiseMap);
  heightMapBuilder.SetDestSize(width + 1, height + 1);
  heightMapBuilder.SetBounds(lowerXBound, upperXBound, lowerZBound, upperZBound);
  heightMapBuilder.Build();

  for (int i = 0; i <= height; i++) {
    for (int j = 0; j <= width; j++) {
      heightMap[(i * (width + 1)) + j] = noiseMap.GetValue(j, i) * 10.0f;
    }
  }
}

void Terrain::generateGeometry(int height, int width, float nearPlane, float nearTopLeft, float tau, float verticalResolution, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext, float cellWidth, float cellDepth) {
  terrainHeight = height;
  terrainWidth = width;
  dX = cellWidth;
  dZ = cellDepth;
  numVertices = (height + 1) * (width + 1);
  numIndices = height * width * 6;

  generateVertices();
  generateIndices(nearPlane, nearTopLeft, tau, verticalResolution, device, immediateContext);
  generateNormals();
  setChunkCentres();
  setNeighbouringChunks();
}

void Terrain::generateVertices() {
  if (!vertices) {
    int k = 0;
    vertices = new SimpleVertex[numVertices];
    float distanceFromCentreZ = ((float) terrainHeight) / 2.0f;
    float x, z;
    for (int i = 0; i <= terrainHeight; i++) {
      float distanceFromCentreX = (((float) terrainWidth) / 2.0f) * -1.0f;
      for (int j = 0; j <= terrainWidth; j++) {
        x = distanceFromCentreX + j;
        z = distanceFromCentreZ - i;
        SimpleVertex temp = SimpleVertex { XMFLOAT3(j * dX + ((-terrainWidth * 0.5f) * dX), getHeight(i, j), -(i * dZ) + ((terrainHeight * 0.5f)) * dZ), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(x, z) };
        k = (i * (terrainWidth + 1)) + j;
        vertices[k] = temp;
      }
    }
  }
}

void Terrain::generateIndices(float nearPlane, float nearTopLeft, float tau, float verticalResolution, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext) {
  for (int i = 0; i < terrainHeight / CHUNK_HEIGHT; i++) {
    for (int j = 0; j < terrainWidth / CHUNK_WIDTH; j++) {
      TerrainChunk* chunk = new TerrainChunk(material, j, i, CHUNK_HEIGHT, CHUNK_WIDTH, terrainWidth, nearPlane, nearTopLeft, tau, verticalResolution, heightMap, device, immediateContext);
      chunk->setPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
      chunk->calcMipMapLevel();
      chunk->update(0);
      chunks.push_back(chunk);
    }
  }
}

void Terrain::generateNormals() {
  int numChunkIndices = CHUNK_HEIGHT * CHUNK_WIDTH * 6;

  for (auto chunk : chunks) {
    for (int i = 0; i < numChunkIndices / 3; i++) {
      UINT index1 = chunk->getIndices()[i * 3];
      UINT index2 = chunk->getIndices()[i * 3 + 1];
      UINT index3 = chunk->getIndices()[i * 3 + 2];

      SimpleVertex& v1 = vertices[index1];
      SimpleVertex& v2 = vertices[index2];
      SimpleVertex& v3 = vertices[index3];

      XMVECTOR v1PosVector = XMLoadFloat3(&v1.posL);
      XMVECTOR v2PosVector = XMLoadFloat3(&v2.posL);
      XMVECTOR v3PosVector = XMLoadFloat3(&v3.posL);
      XMVECTOR v1Tov2 = v2PosVector - v1PosVector;
      XMVECTOR v1Tov3 = v3PosVector - v1PosVector;
      XMVECTOR normal = XMVector3Cross(v1Tov2, v1Tov3);

      XMStoreFloat3(&v1.normL, XMLoadFloat3(&v1.normL) + normal);
      XMStoreFloat3(&v2.normL, XMLoadFloat3(&v2.normL) + normal);
      XMStoreFloat3(&v3.normL, XMLoadFloat3(&v3.normL) + normal);
    }
  }

  for (int i = 0; i < numVertices; i++) {
    XMStoreFloat3(&vertices[i].normL, XMVector3Normalize(XMLoadFloat3(&vertices[i].normL)));
  }
}

void Terrain::setChunkCentres() {
  for (auto chunk : chunks) {
    UINT firstVertexIndex = chunk->getIndices()[0];
    UINT lastVertexIndex = chunk->getIndices()[(CHUNK_HEIGHT * CHUNK_WIDTH * 6) - 1];

    SimpleVertex firstVertex = vertices[firstVertexIndex];
    SimpleVertex lastVertex = vertices[lastVertexIndex];
    XMFLOAT3 firstPos = firstVertex.posL;
    XMFLOAT3 lastPos = lastVertex.posL;
    XMFLOAT3 centre { (firstPos.x + lastPos.x) / 2.0f, (firstPos.y + lastPos.y) / 2.0f, (firstPos.z + lastPos.z) / 2.0f };
    chunk->setCentre(centre);

    float highest = 0;
    float lowest = 0;

    for (int i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH * 6; i++) {
      float height = vertices[chunk->getIndices()[i]].posL.y;

      if (height > highest) {
        highest = height;
      }
      if (height < lowest) {
        lowest = height;
      }
    }

    chunk->setBoundingBox(XMFLOAT3(firstPos.x, highest, firstPos.z), highest - lowest);
  }
}

void Terrain::setNeighbouringChunks() {
  int verticalChunks = terrainHeight / CHUNK_HEIGHT;
  int horizontalChunks = terrainWidth / CHUNK_WIDTH;

  for (int i = 0; i < verticalChunks; i++) {
    for (int j = 0; j < horizontalChunks; j++) {
      if (i > 0) {
        chunks[(i * horizontalChunks) + j]->setNorthChunk(chunks[((i - 1) * horizontalChunks) + j]);
      }
      if (i < verticalChunks - 1) {
        chunks[(i * horizontalChunks) + j]->setSouthChunk(chunks[((i + 1) * horizontalChunks) + j]);
      }
      if (j > 0) {
        chunks[(i * horizontalChunks) + j]->setWestChunk(chunks[(i * horizontalChunks) + j - 1]);
      }
      if (j < horizontalChunks - 1) {
        chunks[(i * horizontalChunks) + j]->setEastChunk(chunks[(i * horizontalChunks) + j + 1]);
      }
    }
  }
}

float Terrain::getHeight(int i, int j) {
  if (heightMap) {
    int index = (i * (terrainWidth + 1)) + j;
    return heightMap[index];
  }
  return 0.0f;
}

float randFloat(float min, float max) {
  int r;
  float	x;

  r = rand();
  x = (float) (r & 0x7fff) / (float) 0x7fff;
  return (x * (max - min) + min);
}

int randInt(int min, int max) {
  return min + (rand() % (int) (max - min + 1));
}

// taken from http://gameprogrammer.com/fractal.html#structure
float avgDiamondVals(int i, int j, int stride, int size, int subSize, float* fa) {
  if (i == 0)
    return ((float) (fa[(i*size) + j - stride] +
    fa[(i*size) + j + stride] +
    fa[((subSize - stride)*size) + j] +
    fa[((i + stride)*size) + j]) * .25f);
  else if (i == size - 1)
    return ((float) (fa[(i*size) + j - stride] +
    fa[(i*size) + j + stride] +
    fa[((i - stride)*size) + j] +
    fa[((0 + stride)*size) + j]) * .25f);
  else if (j == 0)
    return ((float) (fa[((i - stride)*size) + j] +
    fa[((i + stride)*size) + j] +
    fa[(i*size) + j + stride] +
    fa[(i*size) + subSize - stride]) * .25f);
  else if (j == size - 1)
    return ((float) (fa[((i - stride)*size) + j] +
    fa[((i + stride)*size) + j] +
    fa[(i*size) + j - stride] +
    fa[(i*size) + 0 + stride]) * .25f);
  else
    return ((float) (fa[((i - stride)*size) + j] +
    fa[((i + stride)*size) + j] +
    fa[(i*size) + j - stride] +
    fa[(i*size) + j + stride]) * .25f);
}

// taken from http://gameprogrammer.com/fractal.html#structure
float avgSquareVals(int i, int j, int stride, int size, float* fa) {
  return ((float) (fa[((i - stride)*size) + j - stride] +
    fa[((i - stride)*size) + j + stride] +
    fa[((i + stride)*size) + j - stride] +
    fa[((i + stride)*size) + j + stride]) * .25f);
}

float parabola(int centreX, int centreZ, int pointX, int pointZ, int radius) {
  return pow(radius, 2) - (pow(pointX - centreX, 2) + pow(pointZ - centreZ, 2));
}