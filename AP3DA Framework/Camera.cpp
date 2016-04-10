#include "Camera.h"
#include "Terrain.h"

Camera::Camera(bool moveable, bool freeFlying, XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) :
  moveable(moveable), freeFlying(freeFlying), eye(position), baseEye(position), at(at), worldUp(up), up(up), windowWidth(windowWidth), windowHeight(windowHeight)
, nearDepth(nearDepth), farDepth(farDepth) {
  XMStoreFloat3(&right, XMVector4Normalize(XMVector3Cross(XMLoadFloat3(&worldUp), XMLoadFloat3(&at))));
  update();
}

Camera::~Camera() {
}

void Camera::update() {
  // Initialize the view matrix

  if (moveable) {
    XMStoreFloat3(&at, XMVector3Normalize(XMLoadFloat3(&at)));
    if (freeFlying) {
      XMStoreFloat3(&up, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&at), XMLoadFloat3(&right))));
    }
    XMStoreFloat3(&right, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&at))));
  }

  XMFLOAT4 e = XMFLOAT4(eye.x, eye.y, eye.z, 1.0f);
  XMFLOAT4 b = XMFLOAT4(baseEye.x, baseEye.y, baseEye.z, 1.0f);
  XMFLOAT4 a = XMFLOAT4(at.x, at.y, at.z, 1.0f);
  XMFLOAT4 u = XMFLOAT4(worldUp.x, worldUp.y, worldUp.z, 0.0f);

  XMVECTOR eyeVector = XMLoadFloat4(&e);
  XMVECTOR baseVector = XMLoadFloat4(&b);
  XMVECTOR atVector = XMLoadFloat4(&a);
  XMVECTOR upVector = XMLoadFloat4(&u);

  XMStoreFloat4x4(&view, moveable ? XMMatrixLookToLH(eyeVector, atVector, upVector) : XMMatrixLookAtLH(eyeVector, atVector, upVector));

  XMFLOAT4 l = { 0.0f, 0.0f, 1.0f, 0.0f };
  XMVECTOR L = XMLoadFloat4(&l);
  XMVECTOR lookAt = baseVector + L;

  XMStoreFloat4x4(&basicView, XMMatrixLookAtLH(baseVector, lookAt, upVector));

  // Initialize the projection matrix
  XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, windowWidth / windowHeight, nearDepth, farDepth));
  XMStoreFloat4x4(&orthoProjection, XMMatrixOrthographicLH(windowWidth, windowHeight, nearDepth, farDepth));
}

void Camera::reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) {
  windowWidth = windowWidth;
  windowHeight = windowHeight;
  nearDepth = nearDepth;
  farDepth = farDepth;
}

XMFLOAT4X4 Camera::getViewProjection() const {
  XMMATRIX v = XMLoadFloat4x4(&view);
  XMMATRIX p = XMLoadFloat4x4(&projection);

  XMFLOAT4X4 viewProj;

  XMStoreFloat4x4(&viewProj, v * p);

  return viewProj;
}

void Camera::move(const float distance, const Terrain& terrain) {
  if (moveable) {
    XMFLOAT4 temp = XMFLOAT4(at.x, at.y, at.z, 1.0f);

    XMVECTOR trimmedLookDirection = XMLoadFloat4(&temp);
    XMVECTOR moveAmount = XMVectorScale(trimmedLookDirection, distance);
    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat4(&XMFLOAT4(eye.x, eye.y, eye.z, 1.0f)), moveAmount);
    XMStoreFloat3(&eye, newPosition);
    if (!freeFlying) {
      float height = terrain.getCameraHeight(eye.x, eye.z);
      eye.y = height + 2.0f;
    }
  }
}

void Camera::moveY(const float distance) {
  if (moveable && freeFlying) {
    XMVECTOR moveAmount = XMVectorScale(XMLoadFloat4(&XMFLOAT4(worldUp.x, worldUp.y, worldUp.z, 0.0f)), distance);
    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat4(&XMFLOAT4(eye.x, eye.y, eye.z, 1.0f)), moveAmount);
    XMStoreFloat3(&eye, newPosition);
  }
}

void Camera::strafe(const float distance, const Terrain& terrain) {
  if (moveable) {
    XMVECTOR moveAmount = XMVectorScale(XMLoadFloat4(&XMFLOAT4(right.x, right.y, right.z, 0.0f)), distance);
    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat4(&XMFLOAT4(eye.x, eye.y, eye.z, 1.0f)), moveAmount);
    XMStoreFloat3(&eye, newPosition); 
    if (!freeFlying) {
      float height = terrain.getCameraHeight(eye.x, eye.z) - 82.0f;
      eye.y = height + 2.0f;
    }
  }
}

void Camera::rotatePitch(const float angle) {
  if (moveable) {
    XMMATRIX rotationMatrix = XMMatrixRotationAxis(XMLoadFloat4(&XMFLOAT4(right.x, right.y, right.z, 0.0f)), angle);
    XMVECTOR newLookVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(at.x, at.y, at.z, 0.0f)), rotationMatrix);
    XMVECTOR newUpVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(up.x, up.y, up.z, 0.0f)), rotationMatrix);
    XMStoreFloat3(&at, XMVector4Normalize(newLookVector));
    if (freeFlying) {
      XMStoreFloat3(&up, XMVector4Normalize(newUpVector));
    }
  }
}

void Camera::rotateYaw(const float angle) {
  if (moveable) {
    XMMATRIX rotationMatrix = XMMatrixRotationY(angle);
    XMVECTOR newLookVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(at.x, at.y, at.z, 0.0f)), rotationMatrix);
    XMVECTOR newUpVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(up.x, up.y, up.z, 0.0f)), rotationMatrix);
    XMVECTOR newRightVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(right.x, right.y, right.z, 0.0f)), rotationMatrix);
    XMStoreFloat3(&at, XMVector4Normalize(newLookVector));
    if (freeFlying) {
      XMStoreFloat3(&up, XMVector4Normalize(newUpVector));
    }
    XMStoreFloat3(&right, XMVector4Normalize(newRightVector));
  }
}