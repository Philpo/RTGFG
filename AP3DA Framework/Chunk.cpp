#include "Chunk.h"
#include "Application.h"
#include <fstream>

Chunk::Chunk(int height, int width, int depth, Geometry voxelGeometry, Material voxelMaterial, ID3D11Device* d3dDevice, ID3D11DeviceContext * pImmediateContext) :
  GameObject("chunk"), chunkHeight(height), chunkWidth(width), chunkDepth(depth), pImmediateContext(pImmediateContext) {
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

  instanceData.resize(height * width * depth);
  
  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = sizeof(InstanceData) * (height * width * depth);
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  //InstanceData* data = (InstanceData*) InitData.pSysMem;

  //int currentVoxel = 0;
  //for (int i = 0; i < height; i++) {
  //  for (int j = 0; j < width; j++) {
  //    for (int k = 0; k < depth; k++) {
  //      data[currentVoxel] = instanceData[currentVoxel++];
  //    }
  //  }
  //}

  HRESULT hr = d3dDevice->CreateBuffer(&desc, 0, &instanceBuffer);

  if (FAILED(hr)) {
    int a = 1;
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

void Chunk::Update(float t) {
  GameObject::Update(t);

  //for (int i = 0; i < chunkHeight; i++) {
  //  for (int j = 0; j < chunkWidth; j++) {
  //    for (int k = 0; k < chunkDepth; k++) {
  //      voxels[i][j][k]->setVisible(true);

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
  //    }
  //  }
  //}

  int currentVoxel = 0;
  for (int i = 0; i < chunkHeight; i++) {
    for (int j = 0; j < chunkWidth; j++) {
      for (int k = 0; k < chunkDepth; k++) {
        voxels[i][j][k]->Update(t);
        XMStoreFloat4x4(&instanceData[currentVoxel++].world, XMMatrixTranspose(voxels[i][j][k]->GetWorldMatrix()));

        //bool leftVisible, rightVisible, aboveVisible, belowVisible, inFrontVisible, behindVisible;
        //leftVisible = rightVisible = aboveVisible = belowVisible = inFrontVisible = behindVisible = true;

        //if (k > 0 && k < chunkDepth - 1) {
        //  if (!voxels[i][j][k - 1]->isVisible()) {
        //    behindVisible = false;
        //  }
        //  if (!voxels[i][j][k + 1]->isVisible()) {
        //    inFrontVisible = false;
        //  }
        //}
        //if (j > 0 && j < chunkWidth - 1) {
        //  if (!voxels[i][j - 1][k]->isVisible()) {
        //    leftVisible = false;
        //  }
        //  if (!voxels[i][j + 1][k]->isVisible()) {
        //    rightVisible = false;
        //  }
        //}
        //if (i > 0 && i < chunkHeight - 1) {
        //  if (!voxels[i - 1][j][k]->isVisible()) {
        //    belowVisible = false;
        //  }
        //  else if (!voxels[i + 1][j][k]->isVisible()) {
        //    aboveVisible = false;
        //  }
        //}

        //if (!leftVisible || !rightVisible || !aboveVisible || !belowVisible || !inFrontVisible || !behindVisible) {
        //  occlusionMap[i][j][k] = false;
        //}
      }
    }
  }

  //for (int i = 0; i < chunkHeight; i++) {
  //  for (int j = 0; j < chunkWidth; j++) {
  //    for (int k = 0; k < chunkDepth; k++) {
  //      voxels[i][j][k]->setVisible(!occlusionMap[i][j][k]);
  //    }
  //  }
  //}

  D3D11_MAPPED_SUBRESOURCE mappedData;
  pImmediateContext->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
  InstanceData* data = reinterpret_cast<InstanceData*>(mappedData.pData);

  currentVoxel = 0;
  for (int i = 0; i < chunkHeight; i++) {
    for (int j = 0; j < chunkWidth; j++) {
      for (int k = 0; k < chunkDepth; k++) {
        data[currentVoxel] = instanceData[currentVoxel++];
      }
    }
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
  pImmediateContext->DrawIndexedInstanced(voxels[0][0][0]->GetGeometryData().numberOfIndices, chunkHeight * chunkWidth * chunkDepth, 0, 0, 0);


  //for (int i = 0; i < chunkHeight; i++) {
  //  for (int j = 0; j < chunkWidth; j++) {
  //    for (int k = 0; k < chunkDepth; k++) {
  //      if (voxels[i][j][k]->isVisible()) {
  //        Material material = voxels[i][j][k]->GetMaterial();

  //        // Copy material to shader
  //        cb.surface.AmbientMtrl = material.ambient;
  //        cb.surface.DiffuseMtrl = material.diffuse;
  //        cb.surface.SpecularMtrl = material.specular;

  //        // Set world matrix
  //        cb.World = XMMatrixTranspose(voxels[i][j][k]->GetWorldMatrix());

  //        // Set texture
  //        cb.HasTexture = 0.0f;

  //        // Update constant buffer
  //        pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

  //        voxels[i][j][k]->Draw(cb, constantBuffer, pImmediateContext);
  //      }
  //    }
  //  }
  //}
}