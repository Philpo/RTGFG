#include "TerrainChunk.h"
#include <iostream>
#include <math.h>

using namespace std;

TerrainChunk::TerrainChunk(Material material, int xOffset, int zOffset, int height, int width, int terrainWidth, float nearPlane, float nearTopLeft, float tau, float verticalResolution, float* const heightMap, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext) :
GameObject("terrain_chunk", material), xOffset(xOffset), zOffset(zOffset), height(height), width(width), terrainWidth(terrainWidth), visible(true), immediateContext(immediateContext)
, heightMap(heightMap), north(nullptr), south(nullptr), east(nullptr), west(nullptr) {
  numMipLevels = terrainWidth != width ? (terrainWidth / width) - 1 : 1;

  currentMipLevel = 0;
  indices = new UINT*[numMipLevels];
  numberOfIndices = new int[numMipLevels];
  mipMapDistances = new float[numMipLevels];

  for (int i = 0; i < numMipLevels; i++) {
    mipMapDistances[i] = i * width;
    numberOfIndices[i] = i != 0 ? height * width * 6 / (i * 2) : height * width * 6;
    indices[i] = new UINT[numberOfIndices[i]];
  }

  float A = nearPlane / (float) abs(nearTopLeft);
  float T = 2 * tau / (float) verticalResolution;
  kC = A / T;
  cSquared = kC * kC;

  int indicesIndex = 0;
  int heightOffSet = zOffset * height * (terrainWidth + 1);
  int widthOffset = xOffset * width;
  int step = 1;
  for (int level = 0; level < numMipLevels; level++) {
    indicesIndex = 0;
    if (level > 0) {
      step *= 2;
    }

    for (int i = 0; i < height; i += step) {
      for (int j = 0; j < width; j += step) {
        // triangle 1
        indices[level][indicesIndex++] = (i * (terrainWidth + 1)) + j + heightOffSet + widthOffset;
        indices[level][indicesIndex++] = (i * (terrainWidth + 1)) + (j + step) + heightOffSet + widthOffset;
        indices[level][indicesIndex++] = ((i + step) * (terrainWidth + 1)) + j + heightOffSet + widthOffset;
        // triangle 2
        indices[level][indicesIndex++] = ((i + step) * (terrainWidth + 1)) + j + heightOffSet + widthOffset;
        indices[level][indicesIndex++] = (i * (terrainWidth + 1)) + (j + step) + heightOffSet + widthOffset;
        indices[level][indicesIndex++] = ((i + step) * (terrainWidth + 1)) + (j + step) + heightOffSet + widthOffset;
      }
    }
  }

  calcDn2();

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));

  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = sizeof(UINT) * numberOfIndices[0];
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = indices[0];
  device->CreateBuffer(&bd, &InitData, &indexBuffer);
}

TerrainChunk::~TerrainChunk() {
  if (indexBuffer) indexBuffer->Release();
  if (indices) {
    for (int i = 0; i < numMipLevels; i++) {
      delete[] indices[i];
    }
    delete[] indices;
  }
  delete[] numberOfIndices;
}

void TerrainChunk::setBoundingBox(XMFLOAT3 topLeft, float height) {
  boundingBox.topLeft = topLeft;
  boundingBox.width = width;
  boundingBox.height = height;
  boundingBox.depth = this->height;
}

void TerrainChunk::setCameraPosition(XMFLOAT3 cameraPosition) {
  if (this->cameraPosition.x == cameraPosition.x && this->cameraPosition.y == cameraPosition.y && this->cameraPosition.z == cameraPosition.z) {
    cameraMoved = false;
  }
  else {
    cameraMoved = true;
    this->cameraPosition = cameraPosition;
  }
}

void TerrainChunk::calcMipMapLevel() {
  if (visible) {
    if (cameraMoved) {
      XMVECTOR toCamera = XMLoadFloat3(&centre) - XMLoadFloat3(&cameraPosition);
      XMFLOAT3 distanceToCamera;
      XMStoreFloat3(&distanceToCamera, toCamera);

      float d2 = (distanceToCamera.x * distanceToCamera.x) + (distanceToCamera.y * distanceToCamera.y) + (distanceToCamera.z * distanceToCamera.z);

      int newMipLevel = 0;

      while (newMipLevel < numMipLevels - 1 && d2 > mipMapDistances[newMipLevel + 1]) {
        newMipLevel++;
      }

      while (newMipLevel > numMipLevels - 4) {
        newMipLevel--;
      }

      if (newMipLevel != currentMipLevel) {
        currentMipLevel = newMipLevel;
        refresh = true;
      }
      else {
        refresh = false;
      }
    }
  }
}

void TerrainChunk::update(float t) {
  if (visible) {
    GameObject::update(t);

    // if the camera hasn't moved, then we can guarantee that the mip levels haven't changed, so no need to check
    if (cameraMoved) {
      // if any of the neighbouring chunks have changed mip level, recalculate the indices to deal with cracks
      if (north && north->refresh) {
        refresh = true;
      }
      else if (south && south->refresh) {
        refresh = true;
      }
      else if (east && east->refresh) {
        refresh = true;
      }
      else if (west && west->refresh) {
        refresh = true;
      }

      if (refresh) {
        refreshIndexBuffer();
        cameraMoved = false;
      }
    }
  }
}

void TerrainChunk::draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  if (visible) {
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    Material material = getMaterial();

    // Copy material to shader
    cb.surface.ambientMtrl = material.ambient;
    cb.surface.diffuseMtrl = material.diffuse;
    cb.surface.specularMtrl = material.specular;

    // Set texture
    cb.hasTexture = 1.0f;
    cb.world = getWorldMatrix();

    // Update constant buffer
    pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    pImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    pImmediateContext->DrawIndexed(numberOfIndices[currentMipLevel], 0, 0);
  }
}

void TerrainChunk::refreshIndexBuffer() {
  int heightOffSet = zOffset * height * (terrainWidth + 1);
  int widthOffset = xOffset * width;

  D3D11_MAPPED_SUBRESOURCE mappedData;
  immediateContext->Map(indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
  UINT* data = reinterpret_cast<UINT*>(mappedData.pData);

  int scaledIndex;
  for (int i = 0; i < numberOfIndices[currentMipLevel]; i++) {
    scaledIndex = indices[currentMipLevel][i] - heightOffSet - widthOffset;

    if (west && west->currentMipLevel > currentMipLevel && scaledIndex % (terrainWidth + 1) == 0) {
      data[i] = (indices[currentMipLevel][i] & ~west->currentMipLevel);
    }
    else if (east && east->currentMipLevel > currentMipLevel && (scaledIndex - width) % (terrainWidth + 1) == 0) {
      data[i] = (indices[currentMipLevel][i] | east->currentMipLevel);
    }
    else if (north && north->currentMipLevel > currentMipLevel && scaledIndex <= width) {
      data[i] = (indices[currentMipLevel][i] & ~north->currentMipLevel);
    }
    else if (south && south->currentMipLevel > currentMipLevel && scaledIndex >= width * (terrainWidth + 1)) {
      data[i] = (indices[currentMipLevel][i] & ~south->currentMipLevel);
    }
    else {
      data[i] = indices[currentMipLevel][i];
    }
  }

  immediateContext->Unmap(indexBuffer, 0);
}

// taken from https://www.gamedev.net/topic/252983-geomipmaps---precalculating-d/
void TerrainChunk::calcDn2() {
  mipMapDistances[0] = 0;

  float deltaMax = 0;

  for (int l = 0; l < numMipLevels - 2; l++) {
    int start = (int) pow(2.0f, l);
    int inc = start * 2;

    for (int i = start; i < height; i += inc) {
      for (int j = start; j < width; j += inc) {
        deltaMax = max(deltaMax, fabsf(getHeight(i, j) - (getHeight(i - start, j - start) + getHeight(i + start, j + start)) / 2.0f));
        deltaMax = max(deltaMax, fabsf(getHeight(i, j) - (getHeight(i + start, j - start) + getHeight(i - start, j + start)) / 2.0f));
      }
    }
    mipMapDistances[l] = deltaMax * deltaMax * cSquared;
  }
}

float TerrainChunk::getHeight(int i, int j) {
  return heightMap[(i * (terrainWidth + 1)) + j];
}