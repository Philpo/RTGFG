#include "GameObject.h"
#include "Application.h"

GameObject::GameObject(string type) : _type(type) {
  _parent = nullptr;
  _position = XMFLOAT3();
  _rotation = XMFLOAT3();
  originRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
  _scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

  _textureRV = nullptr;
}

GameObject::GameObject(string type, Material material) : _type(type), _material(material) {
  _parent = nullptr;
  _position = XMFLOAT3();
  _rotation = XMFLOAT3();
  originRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
  _scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

  _textureRV = nullptr;
}

GameObject::GameObject(string type, Geometry geometry, Material material) : _geometry(geometry), _type(type), _material(material) {
  _parent = nullptr;
  _position = XMFLOAT3();
  _rotation = XMFLOAT3();
  originRotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
  _scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

  _textureRV = nullptr;
}

GameObject::~GameObject() {
}

void GameObject::update(float t) {
  // Calculate world matrix
  XMMATRIX scale = XMMatrixScaling(_scale.x, _scale.y, _scale.z);
  XMMATRIX rotation = XMMatrixRotationX(_rotation.x) * XMMatrixRotationY(_rotation.y) * XMMatrixRotationZ(_rotation.z);
  XMMATRIX translation = XMMatrixTranslation(_position.x, _position.y, _position.z);
  XMMATRIX originRot = XMMatrixRotationX(originRotation.x) * XMMatrixRotationY(originRotation.y) * XMMatrixRotationZ(originRotation.z);

  XMStoreFloat4x4(&_world, scale * rotation * originRot * translation);

  if (_parent != nullptr) {
    GameObject* parent = _parent;

    while (parent) {
      XMMATRIX temp = XMMatrixIdentity();
      temp *= XMMatrixRotationX(parent->getOriginRotation().x);
      temp *= XMMatrixRotationY(parent->getOriginRotation().y);
      temp *= XMMatrixRotationZ(parent->getOriginRotation().z);
      temp *= XMMatrixTranslation(parent->getPosition().x, parent->getPosition().y, parent->getPosition().z);
      XMStoreFloat4x4(&_world, this->getWorldMatrix() * temp);
      parent = parent->_parent;
    }
  }
}

void GameObject::draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext) {
  // NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

  // Set vertex and index buffers
  pImmediateContext->IASetVertexBuffers(0, 1, &_geometry.vertexBuffer, &_geometry.vertexBufferStride, &_geometry.vertexBufferOffset);
  pImmediateContext->IASetIndexBuffer(_geometry.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

  pImmediateContext->DrawIndexed(_geometry.numberOfIndices, 0, 0);
}
