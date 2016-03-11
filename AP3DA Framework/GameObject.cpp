#include "GameObject.h"
#include "Application.h"

GameObject::GameObject(string type) : _type(type) {
  _parent = nullptr;
  _position = XMFLOAT3();
  _rotation = XMFLOAT3();
  _scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

  _textureRV = nullptr;
}

GameObject::GameObject(string type, Material material) : _type(type), _material(material) {
  _parent = nullptr;
  _position = XMFLOAT3();
  _rotation = XMFLOAT3();
  _scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

  _textureRV = nullptr;
}

GameObject::GameObject(string type, Geometry geometry, Material material) : _geometry(geometry), _type(type), _material(material) {
  _parent = nullptr;
  _position = XMFLOAT3();
  _rotation = XMFLOAT3();
  _scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

  _textureRV = nullptr;
}

GameObject::~GameObject() {
}

void GameObject::Update(float t) {
  // Calculate world matrix
  XMMATRIX scale = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
  XMMATRIX rotation = XMMatrixRotationX(_rotation.x) * XMMatrixRotationY(_rotation.y) * XMMatrixRotationZ(_rotation.z);
  XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);

  XMStoreFloat4x4(&_world, scale * rotation * translation);

  if (_parent != nullptr) {
    XMMATRIX temp = XMMatrixIdentity();
    temp *= XMMatrixRotationX(_parent->GetRotation().x);
    temp *= XMMatrixRotationY(_parent->GetRotation().y);
    temp *= XMMatrixRotationZ(_parent->GetRotation().z);
    temp *= XMMatrixTranslation(_parent->GetPosition().x, _parent->GetPosition().y, _parent->GetPosition().z);
    XMStoreFloat4x4(&_world, this->GetWorldMatrix() * temp);
  }
}

void GameObject::Draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  // NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

  // Set vertex and index buffers
  pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
  pImmediateContext->IASetIndexBuffer(_geometry.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

  pImmediateContext->DrawIndexed(_geometry.numberOfIndices, 0, 0);
}
