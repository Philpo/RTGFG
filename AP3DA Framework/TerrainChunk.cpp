#include "TerrainChunk.h"
#include <iostream>

using namespace std;

TerrainChunk::TerrainChunk(Material material, int xOffset, int zOffset, int height, int width, int terrainWidth, ID3D11Device* const device, ID3D11DeviceContext* const immediateContext) : GameObject("terrain_chunk", material), height(height), width(width), visible(true), immediateContext(immediateContext) {
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
      XMStoreFloat3(&distanceToCamera, XMVector3Length(toCamera));

      int newMipLevel = currentMipLevel;

      for (int i = 0; i < numMipLevels; i++) {
        if (distanceToCamera.x > mipMapDistances[i] && (i == numMipLevels - 1 || distanceToCamera.x <= mipMapDistances[i + 1])) {
          newMipLevel = i;
          break;
        }
      }

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