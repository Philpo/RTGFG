#pragma once

#include <directxmath.h>
#include <d3d11_1.h>
#include <string>
#include "TypeDefs.h"

using namespace DirectX;
using namespace std;

struct Geometry {
  ID3D11Buffer * vertexBuffer;
  ID3D11Buffer * indexBuffer;
  int numberOfIndices;

  UINT vertexBufferStride;
  UINT vertexBufferOffset;
};

struct Material {
  XMFLOAT4 diffuse;
  XMFLOAT4 ambient;
  XMFLOAT4 specular;
  float specularPower;
};

class GameObject {
public:
  GameObject(string type);
  GameObject(string type, Material material);
  GameObject(string type, Geometry geometry, Material material);
  virtual ~GameObject();

  // Setters and Getters for position/rotation/scale
  void setPosition(XMFLOAT3 position) { _position = position; }
  void setPosition(float x, float y, float z) { _position.x = x; _position.y = y; _position.z = z; }

  XMFLOAT3 getPosition() const { return _position; }

  void setScale(XMFLOAT3 scale) { _scale = scale; }
  void setScale(float x, float y, float z) { _scale.x = x; _scale.y = y; _scale.z = z; }

  XMFLOAT3 getScale() const { return _scale; }

  void setRotation(XMFLOAT3 rotation) { _rotation = rotation; }
  void setRotation(float x, float y, float z) { _rotation.x = x; _rotation.y = y; _rotation.z = z; }

  void setOriginRotation(XMFLOAT3 rotation) { originRotation = rotation; }
  void setOriginRotation(float x, float y, float z) { originRotation.x = x; originRotation.y = y; originRotation.z = z; }

  XMFLOAT3 getRotation() const { return _rotation; }
  XMFLOAT3 getOriginRotation() const { return originRotation; }

  string getType() const { return _type; }

  Geometry getGeometryData() const { return _geometry; }

  Material getMaterial() const { return _material; }

  XMMATRIX getWorldMatrix() const { return XMLoadFloat4x4(&_world); }

  void setTextureRV(ID3D11ShaderResourceView* textureRV) { _textureRV = textureRV; }
  ID3D11ShaderResourceView* getTextureRV() const { return _textureRV; }
  bool hasTexture() const { return _textureRV ? true : false; }

  void setParent(GameObject * parent) { _parent = parent; }
  virtual void setGeometry(Geometry geometry) { _geometry = geometry; }

  virtual void update(float t);
  virtual void draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer, ID3D11DeviceContext * pImmediateContext);
protected:
  Geometry _geometry;
private:
  XMFLOAT3 _position;
  XMFLOAT3 _rotation;
  XMFLOAT3 originRotation;
  XMFLOAT3 _scale;

  string _type;

  XMFLOAT4X4 _world;

  Material _material;

  ID3D11ShaderResourceView * _textureRV;

  GameObject * _parent;
};

