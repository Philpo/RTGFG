#include "TerrainChunk.h"
#include <iostream>
#include <math.h>

using namespace std;

TerrainChunk::TerrainChunk(Material material, int xOffset, int zOffset, int height, int width, int terrainWidth, float nearPlane, float nearTopLeft, float tau, float verticalResolution, float* const heightMap, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext) : 
  GameObject("terrain_chunk", material), xOffset(xOffset), zOffset(zOffset), height(height), width(width), terrainWidth(terrainWidth), visible(true), immediateContext(immediateContext), heightMap(heightMap) {
  numMipLevels = terrainWidth != width ? (terrainWidth / width) - 1 : 1;

  //if (numMipLevels > 5) {
  //  numMipLevels = 5;
  //}

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

  float maxHeight = 0;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      maxHeight = max(maxHeight, heightMap[(i * (terrainWidth + 1)) + j + heightOffSet + widthOffset]);
    }
  }

  cout << "chunk (" << xOffset << ", " << zOffset << ") " << maxHeight << endl;

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

void TerrainChunk::Update(float t) {
  if (visible) {
    GameObject::Update(t);

    if (cameraMoved) {
      XMVECTOR toCamera = XMLoadFloat3(&centre) - XMLoadFloat3(&cameraPosition);
      XMFLOAT3 distanceToCamera;
//      XMStoreFloat3(&distanceToCamera, XMVector3Length(toCamera));
      XMStoreFloat3(&distanceToCamera, toCamera);

      float d2 = (distanceToCamera.x * distanceToCamera.x) + (distanceToCamera.y * distanceToCamera.y) + (distanceToCamera.z * distanceToCamera.z);

      int newMipLevel = 0;

      while (newMipLevel < numMipLevels - 1 && d2 > mipMapDistances[newMipLevel + 1]) {
        newMipLevel++;
      }

      if (newMipLevel == numMipLevels - 1) {
        newMipLevel--;
      }

      //int newMipLevel = currentMipLevel;

      //for (int i = 0; i < numMipLevels; i++) {
      //  if (distanceToCamera.x > mipMapDistances[i] && (i == numMipLevels - 1 || distanceToCamera.x <= mipMapDistances[i + 1])) {
      //    newMipLevel = i;
      //    break;
      //  }
      //}

      if (currentMipLevel != newMipLevel) {
        D3D11_MAPPED_SUBRESOURCE mappedData;
        immediateContext->Map(indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
        UINT* data = reinterpret_cast<UINT*>(mappedData.pData);

        for (int i = 0; i < numberOfIndices[newMipLevel]; i++) {
          data[i] = indices[newMipLevel][i];
        }
        immediateContext->Unmap(indexBuffer, 0);
        currentMipLevel = newMipLevel;
      }
    }
  }
}

void TerrainChunk::Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  if (visible) {
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

    Material material = GetMaterial();

    // Copy material to shader
    cb.surface.AmbientMtrl = material.ambient;
    cb.surface.DiffuseMtrl = material.diffuse;
    cb.surface.SpecularMtrl = material.specular;

    // Set texture
    cb.HasTexture = 1.0f;
    cb.World = GetWorldMatrix();

    // Update constant buffer
    pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    pImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    pImmediateContext->DrawIndexed(numberOfIndices[currentMipLevel], 0, 0);
  }
}

void TerrainChunk::calcDn2() {
  mipMapDistances[0] = 0;

  float deltaMax = 0;

  for (int l = 0; l < numMipLevels - 2; l++) {
    int start = (int) pow(2.0f, l);
    int inc = start * 2;
    //first set of removed vertices
    for (int i = start; i < height; i += inc) {
      for (int j = start; j < width; j += inc) {
        deltaMax = max(deltaMax, fabsf(getHeight(i, j) - (getHeight(i - start, j - start) + getHeight(i + start, j + start)) / 2.0f));
        deltaMax = max(deltaMax, fabsf(getHeight(i, j) - (getHeight(i + start, j - start) + getHeight(i - start, j + start)) / 2.0f));
      }
    }
    mipMapDistances[l] = deltaMax * deltaMax * cSquared;
  }

  ////	Loop over each mip-map level.
  //int step = 2;
  //for (int level = 1; level < numMipLevels; ++level) {
  //  //	Loop over all the points in the block calculating deltas.
  //  int endZ = width + zOffset;
  //  int endX = width + xOffset;
  //  for (int i = zOffset; i < endZ; ++i) {
  //    int posZ = i / step;
  //    posZ *= step;
  //    for (int j = xOffset; j < endX; ++j) {
  //      if (i%step != 0 || j%step != 0) {
  //        int posX = j / step;
  //        posX *= step;
  //        float delta = biLinearInterp(posX, posX + step, posZ, posZ + step, j, i);
  //        delta -= heightMap[(i * (terrainWidth + 1)) + j];
  //        delta = abs(delta);
  //        deltaMax = max(deltaMax, delta);
  //      }
  //    }
  //  }

  //  //	Save off maximum delta^2 * C^2;
  //  mipMapDistances[level] = deltaMax * deltaMax * cSquared;


  //  //	Prepare for next level.
  //  step *= 2;
  //}
}

float TerrainChunk::getHeight(int i, int j) {
  return heightMap[(i * (terrainWidth + 1)) + j];
}

float TerrainChunk::biLinearInterp(int lx, int hx, int ly, int hy, int tx, int ty) {
  float s00 = heightMap[(lx * (terrainWidth + 1)) + ly];
  float s01 = heightMap[(hx * (terrainWidth + 1)) + ly];
  float s10 = heightMap[(lx * (terrainWidth + 1)) + hy];
  float s11 = heightMap[(hx * (terrainWidth + 1)) + hy];

  int dx = hx - lx;
  int dtx = tx - lx;
  float v0 = (s01 - s00) / dx*dtx + s00;
  float v1 = (s11 - s10) / dx*dtx + s10;
  float value = (v1 - v0) / (hy - ly)*(ty - ly) + v0;

  return value;
}