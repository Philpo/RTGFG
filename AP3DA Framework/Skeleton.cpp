#include "Skeleton.h"

Skeleton::Skeleton(int numBones, ID3D11Device* d3dDevice, ID3D11DeviceContext* pImmediateContext) : d3dDevice(d3dDevice), pImmediateContext(pImmediateContext), updateInstanceBuffer(true) {
  D3D11_BUFFER_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Usage = D3D11_USAGE_DYNAMIC;
  desc.ByteWidth = sizeof(InstanceData) * numBones;
  desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));

  HRESULT hr = d3dDevice->CreateBuffer(&desc, 0, &instanceBuffer);
}

Skeleton::~Skeleton() {
  for (auto bone : bones) {
    delete bone;
  }
  if (instanceBuffer) instanceBuffer->Release();
}

void Skeleton::runAnimation(const std::vector<KeyFrame>& frames) {
  if (currentFrame < frames.size()) {
    KeyFrame frame = frames[currentFrame];
    GameObject* bone;
    vector<int> effectedBones = frame.getTargets();
    XMFLOAT3 newPos, newRotation;


    if (frameCount <= frame.getNumFrames()) {
      for (int i = 0; i < effectedBones.size(); i++) {
        bone = bones[effectedBones[i]];

        if (frameCount == 1.0f) {
          initialPositions.push_back(bone->GetPosition());
          initialRotations.push_back(bone->GetRotation());
        }

        if (frame.getUpdatePositions()[i]) {
          XMStoreFloat3(&newPos, XMVectorLerp(XMLoadFloat3(&initialPositions[i]), XMLoadFloat3(&frame.getPositions()[i]), frameCount / (float) frame.getNumFrames()));
          bone->SetPosition(newPos);
        }

        if (frame.getUpdateRotations()[i]) {
          XMFLOAT3 rotationInRadians;
          rotationInRadians.x = XMConvertToRadians(frame.getRotations()[i].x);
          rotationInRadians.y = XMConvertToRadians(frame.getRotations()[i].y);
          rotationInRadians.z = XMConvertToRadians(frame.getRotations()[i].z);

          XMStoreFloat3(&newRotation, XMVectorLerp(XMLoadFloat3(&initialRotations[i]), XMLoadFloat3(&rotationInRadians), frameCount / (float) frame.getNumFrames()));
          bone->SetRotation(newRotation);
        }
      }
      frameCount++;
      updateInstanceBuffer = true;
    }
    else {
      currentFrame++;
      frameCount = 1;
      initialPositions.clear();
      initialRotations.clear();
      updateInstanceBuffer = false;
      return;
    }
  }
  else {
    updateInstanceBuffer = false;
  }
}

void Skeleton::update(float t) {
  for (auto bone : bones) {
    bone->Update(t);
  }

  if (updateInstanceBuffer) {
    D3D11_MAPPED_SUBRESOURCE mappedData;
    pImmediateContext->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
    InstanceData* data = reinterpret_cast<InstanceData*>(mappedData.pData);

    for (int i = 0; i < bones.size(); i++) {
      XMStoreFloat4x4(&data[i].world, XMMatrixTranspose(bones[i]->GetWorldMatrix()));
    }
    pImmediateContext->Unmap(instanceBuffer, 0);
  }
}

void Skeleton::draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer) {
  if (bones.size() > 0) {
    UINT stride[2] = { sizeof(SimpleVertex), sizeof(InstanceData) };
    UINT offset[2] = { 0, 0 };

    ID3D11Buffer* buffers[2] = { bones[0]->GetGeometryData().vertexBuffer, instanceBuffer };

    pImmediateContext->IASetVertexBuffers(0, 2, buffers, stride, offset);

    Material material = bones[0]->GetMaterial();

    // Copy material to shader
    cb.surface.AmbientMtrl = material.ambient;
    cb.surface.DiffuseMtrl = material.diffuse;
    cb.surface.SpecularMtrl = material.specular;

    // Set texture
    cb.HasTexture = 0.0f;

    // Update constant buffer
    pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
    pImmediateContext->IASetIndexBuffer(bones[0]->GetGeometryData().indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    pImmediateContext->DrawIndexedInstanced(bones[0]->GetGeometryData().numberOfIndices, bones.size(), 0, 0, 0);
  }

  //if (bones.size() > 0) {
  //  cb.HasTexture = 0.0f;
  //  Material material = bones[0]->GetMaterial();

  //  // Copy material to shader
  //  cb.surface.AmbientMtrl = material.ambient;
  //  cb.surface.DiffuseMtrl = material.diffuse;
  //  cb.surface.SpecularMtrl = material.specular;

  //  for (auto bone : bones) {
  //    cb.World = XMMatrixTranspose(bone->GetWorldMatrix());

  //    pImmediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

  //    bone->Draw(cb, constantBuffer, pImmediateContext);
  //  }
  //}
}