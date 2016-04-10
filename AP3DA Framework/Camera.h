#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "Terrain.h"

using namespace DirectX;

class Camera {
public:
  Camera(bool moveable, bool freeFlying, XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
  ~Camera();

  void update();

  XMFLOAT4X4 getView() const { return view; }
  XMFLOAT4X4 getBasicView() const { return basicView; }
  XMFLOAT4X4 getProjection() const { return projection; }
  XMFLOAT4X4 getOrthoProjection() const { return orthoProjection; }

  XMFLOAT4X4 getViewProjection() const;

  XMFLOAT3 getPosition() const { return eye; }
  XMFLOAT3 getLookAt() const { return at; }
  XMFLOAT3 getUp() const { return worldUp; }
  XMFLOAT3 getRight() const { return right; }

  void setPosition(XMFLOAT3 position) { eye = position; }
  void setLookAt(XMFLOAT3 lookAt) { at = lookAt; }
  void setUp(XMFLOAT3 up) { worldUp = up; }

  void move(const float distance, const Terrain& terrain);
  void moveY(const float distance);
  void strafe(const float distance, const Terrain& terrain);
  void rotatePitch(const float angle);
  void rotateYaw(const float angle);

  void reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
private:
  bool moveable, freeFlying;

  XMFLOAT3 eye, baseEye;
  XMFLOAT3 at;
  XMFLOAT3 worldUp;
  XMFLOAT3 right, up;

  FLOAT windowWidth;
  FLOAT windowHeight;
  FLOAT nearDepth;
  FLOAT farDepth;

  XMFLOAT4X4 view, basicView;
  XMFLOAT4X4 projection, orthoProjection;
};