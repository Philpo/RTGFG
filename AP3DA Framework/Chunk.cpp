#include "Chunk.h"
#include "Application.h"
#include <math.h>
#include <fstream>

Chunk::Chunk(int height, int width, int depth, Geometry voxelGeometry, Material voxelMaterial, ID3D11Device* d3dDevice, ID3D11DeviceContext * pImmediateContext) :
GameObject("chunk"), chunkHeight(height), chunkWidth(width), chunkDepth(depth), pImmediateContext(pImmediateContext), d3dDevice(d3dDevice) {
  occlusionMap = new bool**[height];
  for (int i = 0; i < height; i++) {
    occlusionMap[i] = new bool*[width];
    for (int j = 0; j < width; j++) {
      occlusionMap[i][j] = new bool[depth];
    }
  }

  voxels = new Voxel***[height];
  for (int i = 0; i < height; i++) {
    voxels[i] = new Voxel**[width];
    for (int j = 0; j < width; j++) {
      voxels[i][j] = new Voxel*[depth];
      for (int k = 0; k < depth; k++) {
        Voxel* voxel = new Voxel(voxelGeometry, voxelMaterial);
        voxel->SetParent(this);
        voxel->SetPosition(-chunkWidth + (j * 2), -chunkHeight + (i * 2), -chunkDepth + (k * 2));
        voxels[i][j][k] = voxel;
      }
    }
  }
}

Chunk::~Chunk() {
  ofstream file("occlusion.txt");

  for (int i = 0; i < chunkHeight; i++) {
    for (int j = 0; j < chunkWidth; j++) {
      for (int k = 0; k < chunkDepth; k++) {
        file << occlusionMap[i][j][k];
      }
      file << endl;
      delete[] occlusionMap[i][j];
    }
    file << endl << endl;
    delete[] occlusionMap[i];
  }
  delete[] occlusionMap;
  file.close();

  for (int i = 0; i < chunkHeight; i++) {
    for (int j = 0; j < chunkWidth; j++) {
      for (int k = 0; k < chunkDepth; k++) {
        delete voxels[i][j][k];
      }
      delete[] voxels[i][j];
    }
    delete[] voxels[i];
  }
  delete[] voxels;
}

void Chunk::perlinNoise2D(int seed, int width, int height, int octaves, float persistence, int zoom) {
  float x, y;
  int floorx, floory;
  float na, nb, nc, nd;
  float la, lb, lc;
  float frequency, amplitude;

  heightMap2D = new float*[height];

  for (int i = 0; i < height; i++) {
    heightMap2D[i] = new float[width];
    for (int j = 0; j < width; j++) {
      heightMap2D[i][j] = 0.0f;
    }
  }

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      for (int o = 0; o <= octaves; o++) {
        frequency = (float) pow(2, o);
        amplitude = (float) pow(persistence, o);

        x = ((float) j) * frequency / zoom;
        y = ((float) i) / zoom * frequency;
        floorx = (int) x;
        floory = (int) y;
        na = random(seed, floorx, floory, 0);
        nb = random(seed, floorx + 1, floory, 0);
        nc = random(seed, floorx, floory + 1, 0);
        nd = random(seed, floorx + 1, floory + 1, 0);
        la = lerp(na, nb, x - floorx);
        lb = lerp(nc, nd, x - floorx);
        lc = lerp(la, lb, y - floory);

        heightMap2D[i][j] += lc * amplitude;
      }
    }
  }
}

void Chunk::perlinNoise3D(int seed, int width, int height, int depth, int octaves, float persistence, int zoom) {
  int floorx, floory, floorz;
  float x, y, z;
  float na, nb, nc, nd, ne, nf, ng, nh;
  float la, lb, lc, ld, le, lf;
  int ixl, ixh, iyl, iyh, izl, izh;
  float frequency, amplitude;

  float*** noiseVolume = new float**[height];
  heightMap3D = new float**[height];

  for (int i = 0; i < height; i++) {
    heightMap3D[i] = new float*[width];
    noiseVolume[i] = new float*[width];
    for (int j = 0; j < width; j++) {
      heightMap3D[i][j] = new float[depth];
      noiseVolume[i][j] = new float[depth];
      for (int k = 0; k < depth; k++) {
        heightMap3D[i][j][k] = 0.0f;
        noiseVolume[i][j][k] = random(seed, i, j, k);
      }
    }
  }

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      for (int k = 0; k < depth; k++) {
        for (int o = 0; o <= octaves; o++) {
          frequency = (float) pow(2, o);
          amplitude = (float) pow(persistence, o);

          x = ((float) j) * frequency / zoom;
          z = ((float) i) / zoom * frequency;
          y = ((float) k) * frequency / zoom;

          floorx = (int) x;
          floory = (int) y;
          floorz = (int) z;

          ixl = floorx % width;
          ixh = (floorx + 1) % width;
          iyl = floory % width;
          iyh = (floory + 1) % width;
          izl = floorz % width;
          izh = (floorz + 1) % width;

          na = noiseVolume[ixl][iyl][izl];
          nb = noiseVolume[ixh][iyl][izl];
          nc = noiseVolume[ixl][iyh][izl];
          nd = noiseVolume[ixh][iyh][izl];
          ne = noiseVolume[ixl][iyl][izh];
          nf = noiseVolume[ixh][iyl][izh];
          ng = noiseVolume[ixl][iyh][izh];
          nh = noiseVolume[ixh][iyh][izh];

          la = lerp(na, nb, x - floorx);
          lb = lerp(nc, nd, x - floorx);
          lc = lerp(ne, nf, x - floorx);
          ld = lerp(ng, nh, x - floorx);
          le = lerp(la, lb, y - floory);
          lf = lerp(lc, ld, y - floory);

          heightMap3D[i][j][k] += lerp(le, lf, z - floorz) * amplitude;
        }
      }
    }
  }
}

float Chunk::random(int seed, int x, int y, int z) {
  int n = seed + x * 73 + y * 179 + z * 283;
  n = (n << 13) ^ n;
  int m = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
  return 1.0f - ((float) m / 1073741824.0f);
}

float Chunk::lerp(float a, float b, float x) {
  float ft = x * 3.1415927f;
  float f = (1.0f - (float) cos(ft)) * 0.5f;
  return a * (1.0f - f) + b * f;
}

void Chunk::Update(float t) {
  GameObject::Update(t);

  //for (int i = 0; i < chunkHeight; i++) {
  //  for (int j = 0; j < chunkWidth; j++) {
  //    for (int k = 0; k < chunkDepth; k++) {
  //      if (i == 0 || i == chunkHeight - 1) {
  //        occlusionMap[i][j][k] = false;
  //      }
  //      else if (j == 0 || j == chunkWidth - 1) {
  //        occlusionMap[i][j][k] = false;
  //      }
  //      else if (k == 0 || k == chunkDepth - 1) {
  //        occlusionMap[i][j][k] = false;
  //      }
  //      else {
  //        occlusionMap[i][j][k] = true;
  //      }
  //      voxels[i][j][k]->setVisible(!occlusionMap[i][j][k]);
  //    }
  //  }
  //}

  int currentVoxel = 0;
  visibleVoxels = 0;
  for (int i = 0; i < chunkHeight; i++) {
    for (int j = 0; j < chunkWidth; j++) {
      for (int k = 0; k < chunkDepth; k++) {
        voxels[i][j][k]->Update(t);
        cout << heightMap2D[k][j] << endl;
        if (voxels[i][j][k]->GetPosition().y > heightMap2D[k][j]) {
          voxels[i][j][k]->setVisible(false);
        }

        if (voxels[i][j][k]->GetPosition().y > heightMap3D[k][j][i]) {
          voxels[i][j][k]->setVisible(false);
        }

        if (voxels[i][j][k]->isVisible()) {
          visibleVoxels++;
        }
      }
    }
  }

  instanceData.resize(visibleVoxels);
  for (int i = 0; i < chunkHeight; i++) {
    for (int j = 0; j < chunkWidth; j++) {
      for (int k = 0; k < chunkDepth; k++) {
        if (voxels[i][j][k]->isVisible()) {
          XMStoreFloat4x4(&instanceData[currentVoxel++].world, XMMatrixTranspose(voxels[i][j][k]->GetWorldMatrix()));
        }
      }
    }
  }

  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = sizeof(InstanceData) * visibleVoxels;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));

  HRESULT hr = d3dDevice->CreateBuffer(&desc, 0, &instanceBuffer);

  if (FAILED(hr)) {
    int a = 1;
  }

  D3D11_MAPPED_SUBRESOURCE mappedData;
  pImmediateContext->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
  InstanceData* data = reinterpret_cast<InstanceData*>(mappedData.pData);

  for (int i = 0; i < instanceData.size(); i++) {
    data[i] = instanceData[i];
  }
  pImmediateContext->Unmap(instanceBuffer, 0);
}

void Chunk::Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  UINT stride[2] = { sizeof(SimpleVertex), sizeof(InstanceData) };
  UINT offset[2] = { 0, 0 };

  ID3D11Buffer* buffers[2] = { voxels[0][0][0]->GetGeometryData().vertexBuffer, instanceBuffer };

  pImmediateContext->IASetVertexBuffers(0, 2, buffers, stride, offset);

  Material material = voxels[0][0][0]->GetMaterial();

  // Copy material to shader
  cb.surface.AmbientMtrl = material.ambient;
  cb.surface.DiffuseMtrl = material.diffuse;
  cb.surface.SpecularMtrl = material.specular;

  // Set texture
  cb.HasTexture = 0.0f;

  // Update constant buffer
  pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
  pImmediateContext->IASetIndexBuffer(voxels[0][0][0]->GetGeometryData().indexBuffer, DXGI_FORMAT_R32_UINT, 0);
  pImmediateContext->DrawIndexedInstanced(voxels[0][0][0]->GetGeometryData().numberOfIndices, visibleVoxels, 0, 0, 0);
}