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
  if (heightMap2) {
    for (int i = 0; i < terrainWidth; i++) {
      delete[] heightMap2;
    }
    delete[] heightMap2;
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

void Terrain::frustumCull(XMFLOAT4 leftPlane, XMFLOAT4 rightPlane, XMFLOAT4 topPlane, XMFLOAT4 bottomPlane, XMFLOAT4 nearPlane, XMFLOAT4 farPlane) {
  XMVECTOR dotProduct;
  XMFLOAT3 temp;
  XMFLOAT4 distanceFromLeftTop, distanceFromRightTop, distanceFromTopTop, distanceFromBottomTop, distanceFromNearTop, distanceFromFarTop;
  XMFLOAT4 distanceFromLeftBottom, distanceFromRightBottom, distanceFromTopBottom, distanceFromBottomBottom, distanceFromNearBottom, distanceFromFarBottom;
  XMFLOAT3 trimmedLeft, trimmedRight, trimmedTop, trimmedBottom, trimmedNear, trimmedFar;
  BoundingBox boundingBox;

  trimmedLeft = XMFLOAT3(leftPlane.x, leftPlane.y, leftPlane.z);
  trimmedRight = XMFLOAT3(rightPlane.x, rightPlane.y, rightPlane.z);
  trimmedTop = XMFLOAT3(topPlane.x, topPlane.y, topPlane.z);
  trimmedBottom = XMFLOAT3(bottomPlane.x, bottomPlane.y, bottomPlane.z);
  trimmedNear = XMFLOAT3(nearPlane.x, nearPlane.y, nearPlane.z);
  trimmedFar = XMFLOAT3(farPlane.x, farPlane.y, farPlane.z);

  for (auto chunk : chunks) {
    boundingBox = chunk->getBoundingBox();

    // top top left corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftTop.x = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedRight)));
    distanceFromRightTop.x = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedTop)));
    distanceFromTopTop.x = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomTop.x = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedNear)));
    distanceFromNearTop.x = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&boundingBox.topLeft), XMLoadFloat3(&trimmedFar)));
    distanceFromFarTop.x = temp.x + farPlane.w;

    // top top right corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftTop.y = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightTop.y = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopTop.y = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomTop.y = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearTop.y = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarTop.y = temp.x + farPlane.w;

    // top bottom left corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftTop.z = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightTop.z = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopTop.z = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomTop.z = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearTop.z = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarTop.z = temp.x + farPlane.w;

    // top bottom right corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftTop.w = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightTop.w = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopTop.w = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomTop.w = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearTop.w = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarTop.w = temp.x + farPlane.w;

    // bottom top left corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftBottom.x = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightBottom.x = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopBottom.x = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomBottom.x = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearBottom.x = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarBottom.x = temp.x + farPlane.w;

    // bottom top right corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftBottom.y = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightBottom.y = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopBottom.y = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomBottom.y = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearBottom.y = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarBottom.y = temp.x + farPlane.w;

    // bottom bottom left corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftBottom.z = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightBottom.z = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopBottom.z = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomBottom.z = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearBottom.z = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarBottom.z = temp.x + farPlane.w;

    // bottom bottom right corner
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedLeft)));
    distanceFromLeftBottom.w = temp.x + leftPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedRight)));
    distanceFromRightBottom.w = temp.x + rightPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedTop)));
    distanceFromTopBottom.w = temp.x + topPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedBottom)));
    distanceFromBottomBottom.w = temp.x + bottomPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedNear)));
    distanceFromNearBottom.w = temp.x + nearPlane.w;
    XMStoreFloat3(&temp, XMVector3Dot(XMLoadFloat3(&XMFLOAT3(boundingBox.topLeft.x + boundingBox.width, boundingBox.topLeft.y - boundingBox.height, boundingBox.topLeft.z - boundingBox.depth)), XMLoadFloat3(&trimmedFar)));
    distanceFromFarBottom.w = temp.x + farPlane.w;

    if (distanceFromLeftTop.x > 0 && distanceFromLeftTop.y > 0 && distanceFromLeftTop.z > 0 && distanceFromLeftTop.w > 0 && distanceFromLeftBottom.x > 0 && distanceFromLeftBottom.y > 0 && distanceFromLeftBottom.z > 0 && distanceFromLeftBottom.w > 0) {
      chunk->setVisible(false);
      continue;
    }
    if (distanceFromLeftTop.x > 0 && distanceFromLeftTop.y > 0 && distanceFromLeftTop.z > 0 && distanceFromLeftTop.w > 0 && distanceFromLeftBottom.x > 0 && distanceFromLeftBottom.y > 0 && distanceFromLeftBottom.z > 0 && distanceFromLeftBottom.w > 0) {
      chunk->setVisible(false);
      continue;
    }
    if (distanceFromLeftTop.x > 0 && distanceFromLeftTop.y > 0 && distanceFromLeftTop.z > 0 && distanceFromLeftTop.w > 0 && distanceFromLeftBottom.x > 0 && distanceFromLeftBottom.y > 0 && distanceFromLeftBottom.z > 0 && distanceFromLeftBottom.w > 0) {
      chunk->setVisible(false);
      continue;
    }
    if (distanceFromLeftTop.x > 0 && distanceFromLeftTop.y > 0 && distanceFromLeftTop.z > 0 && distanceFromLeftTop.w > 0 && distanceFromLeftBottom.x > 0 && distanceFromLeftBottom.y > 0 && distanceFromLeftBottom.z > 0 && distanceFromLeftBottom.w > 0) {
      chunk->setVisible(false);
      continue;
    }
    if (distanceFromLeftTop.x > 0 && distanceFromLeftTop.y > 0 && distanceFromLeftTop.z > 0 && distanceFromLeftTop.w > 0 && distanceFromLeftBottom.x > 0 && distanceFromLeftBottom.y > 0 && distanceFromLeftBottom.z > 0 && distanceFromLeftBottom.w > 0) {
      chunk->setVisible(false);
      continue;
    }
    if (distanceFromLeftTop.x > 0 && distanceFromLeftTop.y > 0 && distanceFromLeftTop.z > 0 && distanceFromLeftTop.w > 0 && distanceFromLeftBottom.x > 0 && distanceFromLeftBottom.y > 0 && distanceFromLeftBottom.z > 0 && distanceFromLeftBottom.w > 0) {
      chunk->setVisible(false);
      continue;
    }

    chunk->setVisible(true);
  }
}

void Terrain::Update(float t) {
  for (auto chunk : chunks) {
    chunk->Update(t);
  }
}

void Terrain::Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
  for (auto chunk : chunks) {
    chunk->Draw(cb, constantBuffer, pImmediateContext);
  }
}

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
    heightMap[i] = in[i];//(in[i] / 255.0f);// * heightScale;
  }
}

void Terrain::diamondSquare(int size, int seedValue, float heightScale, float h) {
  int	stride, firstLine, subSize, numIterations = 0;
  float ratio, scale;

  subSize = size;
  size++;
  heightMap = new float[size * size];
  for (int i = 0; i < size * size; i++) {
    heightMap[i] = 0.0f;
  }
  srand(seedValue);

  /* Set up our roughness constants.
  Random numbers are always generated in the range 0.0 to 1.0.
  'scale' is multiplied by the randum number.
  'ratio' is multiplied by 'scale' after each iteration
  to effectively reduce the randum number range.
  */
  ratio = (float) pow(2.0, -h);
  scale = heightScale * ratio;

  /* Seed the first four values. For example, in a 4x4 array, we
  would initialize the data points indicated by '*':

  *   .   .   .   *

  .   .   .   .   .

  .   .   .   .   .

  .   .   .   .   .

  *   .   .   .   *

  In terms of the "diamond-square" algorithm, this gives us
  "squares".

  We want the four corners of the array to have the same
  point. This will allow us to tile the arrays next to each other
  such that they join seemlessly. */

  stride = subSize / 2;
  //heightMap[(0 * size) + 0] = heightMap[(subSize*size) + 0] = heightMap[(subSize*size) + subSize] = heightMap[(0 * size) + subSize] = 0.0f;

  /* Now we add ever-increasing detail based on the "diamond" seeded
  values. We loop over stride, which gets cut in half at the
  bottom of the loop. Since it's an int, eventually division by 2
  will produce a zero result, terminating the loop. */
  while (stride) {
    cout << stride << endl;
    numIterations++;
    /* Take the existing "square" data and produce "diamond"
    data. On the first pass through with a 4x4 matrix, the
    existing data is shown as "X"s, and we need to generate the
    "*" now:

    X   .   .   .   X

    .   .   .   .   .

    .   .   *   .   .

    .   .   .   .   .

    X   .   .   .   X

    It doesn't look like diamonds. What it actually is, for the
    first pass, is the corners of four diamonds meeting at the
    center of the array. */
    for (int i = stride; i < subSize; i += stride) {
      for (int j = stride; j < subSize; j += stride) {
        heightMap[(i * size) + j] = randFloat(-h, h) + avgSquareVals(i, j, stride, size, heightMap);
        j += stride;
      }
      i += stride;
    }

    /* Take the existing "diamond" data and make it into
    "squares". Back to our 4X4 example: The first time we
    encounter this code, the existing values are represented by
    "X"s, and the values we want to generate here are "*"s:

    X   .   *   .   X

    .   .   .   .   .

    *   .   X   .   *

    .   .   .   .   .

    X   .   *   .   X

    i and j represent our (x,y) position in the array. The
    first value we want to generate is at (i=2,j=0), and we use
    "oddline" and "stride" to increment j to the desired value.
    */
    firstLine = 0;
    for (int i = 0; i < subSize; i += stride) {
      firstLine = (firstLine == 0);
      for (int j = 0; j < subSize; j += stride) {
        if (firstLine && !j) {
          j += stride;
        }

        /* i and j are setup. Call avgDiamondVals with the
        current position. It will return the average of the
        surrounding diamond data points. */
        heightMap[(i * size) + j] = randFloat(-h, h) + avgDiamondVals(i, j, stride, size, subSize, heightMap);

        /* To wrap edges seamlessly, copy edge values around
        to other side of array */
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

  cout << numIterations << endl;
  ofstream a("diamond_square.txt");
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      a << heightMap[(i * size) + j] << " ";
    }
    a << endl;
  }
  a.close();
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
  //heightMapBuilder.SetSourceModule(mountainTerrain);
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
        SimpleVertex temp = SimpleVertex{ XMFLOAT3(j * dX + ((-terrainWidth * 0.5f) * dX), getHeight(i, j), -(i * dZ) + ((terrainHeight * 0.5f)) * dZ), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(x, z) };
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
      chunk->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
      chunk->Update(0);
      chunks.push_back(chunk);
    }
  }
  //if (!indices) {
  //  indices = new UINT[numIndices];
  //  int indicesIndex = 0;
  //  for (int i = 0; i < terrainHeight; i++) {
  //    for (int j = 0; j < terrainWidth; j++) {
  //      // triangle 1
  //      indices[indicesIndex++] = (i * (terrainWidth + 1)) + j;
  //      indices[indicesIndex++] = (i * (terrainWidth + 1)) + j + 1;
  //      indices[indicesIndex++] = ((i + 1) * (terrainWidth + 1)) + j;
  //      // triangle 2
  //      indices[indicesIndex++] = ((i + 1) * (terrainWidth + 1)) + j;
  //      indices[indicesIndex++] = (i * (terrainWidth + 1)) + j + 1;
  //      indices[indicesIndex++] = ((i + 1) * (terrainWidth + 1)) + j + 1;
  //    }
  //  }
  //}
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

      XMVECTOR v1PosVector = XMLoadFloat3(&v1.PosL);
      XMVECTOR v2PosVector = XMLoadFloat3(&v2.PosL);
      XMVECTOR v3PosVector = XMLoadFloat3(&v3.PosL);
      XMVECTOR v1Tov2 = v2PosVector - v1PosVector;
      XMVECTOR v1Tov3 = v3PosVector - v1PosVector;
      XMVECTOR normal = XMVector3Cross(v1Tov2, v1Tov3);

      XMStoreFloat3(&v1.NormL, XMLoadFloat3(&v1.NormL) + normal);
      XMStoreFloat3(&v2.NormL, XMLoadFloat3(&v2.NormL) + normal);
      XMStoreFloat3(&v3.NormL, XMLoadFloat3(&v3.NormL) + normal);
    }
  }

  for (int i = 0; i < numVertices; i++) {
    XMStoreFloat3(&vertices[i].NormL, XMVector3Normalize(XMLoadFloat3(&vertices[i].NormL)));
  }
}

void Terrain::setChunkCentres() {
  for (auto chunk : chunks) {
    UINT firstVertexIndex = chunk->getIndices()[0];
    UINT lastVertexIndex = chunk->getIndices()[(CHUNK_HEIGHT * CHUNK_WIDTH * 6) - 1];

    SimpleVertex firstVertex = vertices[firstVertexIndex];
    SimpleVertex lastVertex = vertices[lastVertexIndex];
    XMFLOAT3 firstPos = firstVertex.PosL;
    XMFLOAT3 lastPos = lastVertex.PosL;
    XMFLOAT3 centre { (firstPos.x + lastPos.x) / 2.0f, (firstPos.y + lastPos.y) / 2.0f, (firstPos.z + lastPos.z) / 2.0f };
    chunk->setCentre(centre);

    float highest = 0;
    float lowest = 0;

    for (int i = 0; i < CHUNK_HEIGHT * CHUNK_WIDTH * 6; i++) {
      float height = vertices[chunk->getIndices()[i]].PosL.y;
      
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

float Terrain::getHeight(int i, int j) {
  if (heightMap) {
    int index = (i * (terrainWidth + 1)) + j;
    return heightMap[index];
  }
  return 0.0f;
}

float Terrain::getHeight2(int i, int j) {
  if (heightMap2[i]) {
    return heightMap2[i][j];
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

float avgDiamondVals(int i, int j, int stride, int size, int subSize, float* fa) {
  /* In this diagram, our input stride is 1, the i,j location is
  indicated by "X", and the four value we want to average are
  "*"s:
  .   *   .

  *   X   *

  .   *   .
  */

  /* In order to support tiled surfaces which meet seamless at the
  edges (that is, they "wrap"), We need to be careful how we
  calculate averages when the i,j diamond center lies on an edge
  of the array. The first four 'if' clauses handle these
  cases. The final 'else' clause handles the general case (in
  which i,j is not on an edge).
  */
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


/*
* avgSquareVals - Given the i,j location as the center of a square,
* average the data values at the four corners of the square and return
* it. "Stride" represents half the length of one side of the square.
*
* Called by fill2DFractArray.
*/
float avgSquareVals(int i, int j, int stride, int size, float* fa) {
  /* In this diagram, our input stride is 1, the i,j location is
  indicated by "*", and the four value we want to average are
  "X"s:
  X   .   X

  .   *   .

  X   .   X
  */
  return ((float) (fa[((i - stride)*size) + j - stride] +
    fa[((i - stride)*size) + j + stride] +
    fa[((i + stride)*size) + j - stride] +
    fa[((i + stride)*size) + j + stride]) * .25f);
}

float parabola(int centreX, int centreZ, int pointX, int pointZ, int radius) {
  return pow(radius, 2) - (pow(pointX - centreX, 2) + pow(pointZ - centreZ, 2));
}