#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "Terrain.h"

using namespace DirectX;

class Camera {
private:
  bool moveable, freeFlying;

  XMFLOAT3 _eye, baseEye;
  XMFLOAT3 _at;
  XMFLOAT3 _worldUp;
  XMFLOAT3 right, up;

  FLOAT _windowWidth;
  FLOAT _windowHeight;
  FLOAT _nearDepth;
  FLOAT _farDepth;

  XMFLOAT4X4 _view, basicView;
  XMFLOAT4X4 _projection, orthoProjection;

public:
  Camera(bool moveable, bool freeFlying, XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
  ~Camera();

  void Update();

  XMFLOAT4X4 GetView() const { return _view; }
  XMFLOAT4X4 GetBasicView() const { return basicView; }
  XMFLOAT4X4 GetProjection() const { return _projection; }
  XMFLOAT4X4 getOrthoProjection() const { return orthoProjection; }

  XMFLOAT4X4 GetViewProjection() const;

  XMFLOAT3 GetPosition() const { return _eye; }
  XMFLOAT3 GetLookAt() const { return _at; }
  XMFLOAT3 GetUp() const { return _worldUp; }

  void SetPosition(XMFLOAT3 position) { _eye = position; }
  void SetLookAt(XMFLOAT3 lookAt) { _at = lookAt; }
  void SetUp(XMFLOAT3 up) { _worldUp = up; }
//  void setHeight(float height);

  void move(const float distance, const Terrain& terrain);
  void moveY(const float distance);
  void strafe(const float distance, const Terrain& terrain);
  void rotatePitch(const float angle);
  void rotateYaw(const float angle);

  void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
};

