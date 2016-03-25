#include "Camera.h"
#include "Terrain.h"

Camera::Camera(bool moveable, bool freeFlying, XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) :
  moveable(moveable), freeFlying(freeFlying), _eye(position), baseEye(position), _at(at), _worldUp(up), up(up), _windowWidth(windowWidth), _windowHeight(windowHeight), _nearDepth(nearDepth), _farDepth(farDepth) {
  XMStoreFloat3(&right, XMVector4Normalize(XMVector3Cross(XMLoadFloat3(&_worldUp), XMLoadFloat3(&_at))));
  Update();
}

Camera::~Camera() {
}

void Camera::Update() {
  // Initialize the view matrix

  if (moveable) {
    XMStoreFloat3(&_at, XMVector3Normalize(XMLoadFloat3(&_at)));
    if (freeFlying) {
      XMStoreFloat3(&up, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&_at), XMLoadFloat3(&right))));
      //up = XMVector4Normalize(XMVector3Cross(lookDirection, right));
    }
    XMStoreFloat3(&right, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&_at))));
    //right = XMVector3Cross(up, lookDirection);
    //viewMatrix = XMMatrixLookToLH(position, lookDirection, worldUp);
  }

  XMFLOAT4 eye = XMFLOAT4(_eye.x, _eye.y, _eye.z, 1.0f);
  XMFLOAT4 base = XMFLOAT4(baseEye.x, baseEye.y, baseEye.z, 1.0f);
  XMFLOAT4 at = XMFLOAT4(_at.x, _at.y, _at.z, 1.0f);
  XMFLOAT4 up = XMFLOAT4(_worldUp.x, _worldUp.y, _worldUp.z, 0.0f);

  XMVECTOR EyeVector = XMLoadFloat4(&eye);
  XMVECTOR baseVector = XMLoadFloat4(&base);
  XMVECTOR AtVector = XMLoadFloat4(&at);
  XMVECTOR UpVector = XMLoadFloat4(&up);

  XMStoreFloat4x4(&_view, moveable ? XMMatrixLookToLH(EyeVector, AtVector, UpVector) : XMMatrixLookAtLH(EyeVector, AtVector, UpVector));

  XMFLOAT4 l = { 0.0f, 0.0f, 1.0f, 0.0f };
  XMVECTOR L = XMLoadFloat4(&l);
  XMVECTOR lookAt = baseVector + L;

  XMStoreFloat4x4(&basicView, XMMatrixLookAtLH(baseVector, lookAt, UpVector));

  // Initialize the projection matrix
  XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / _windowHeight, _nearDepth, _farDepth));
  XMStoreFloat4x4(&orthoProjection, XMMatrixOrthographicLH(_windowWidth, _windowHeight, _nearDepth, _farDepth));
}

void Camera::Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth) {
  _windowWidth = windowWidth;
  _windowHeight = windowHeight;
  _nearDepth = nearDepth;
  _farDepth = farDepth;
}

XMFLOAT4X4 Camera::GetViewProjection() const {
  XMMATRIX view = XMLoadFloat4x4(&_view);
  XMMATRIX projection = XMLoadFloat4x4(&_projection);

  XMFLOAT4X4 viewProj;

  XMStoreFloat4x4(&viewProj, view * projection);

  return viewProj;
}

//void Camera::setHeight(float height) {
//  if (moveable && !freeFlying) {
//    _at
//  }
//}

void Camera::move(const float distance, const Terrain& terrain) {
  if (moveable) {
    XMFLOAT4 temp = XMFLOAT4(_at.x, _at.y, _at.z, 1.0f);
    //if (!freeFlying) {
    //  // set the y component of the camera's look vector to 0, since we want to move along the ground plane, rather than free flying
    //  temp.y = 0.0f;
    //}
    XMVECTOR trimmedLookDirection = XMLoadFloat4(&temp);
    XMVECTOR moveAmount = XMVectorScale(trimmedLookDirection, distance);
    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat4(&XMFLOAT4(_eye.x, _eye.y, _eye.z, 1.0f)), moveAmount);
    XMStoreFloat3(&_eye, newPosition);
    if (!freeFlying) {
      float height = terrain.getCameraHeight(_eye.x, _eye.z);
      _eye.y = height + 2.0f;
      //_at.y += height;
    }
    //rebuildViewMatrix();
  }
}

void Camera::moveY(const float distance) {
  if (moveable && freeFlying) {
    XMVECTOR moveAmount = XMVectorScale(XMLoadFloat4(&XMFLOAT4(_worldUp.x, _worldUp.y, _worldUp.z, 0.0f)), distance);
    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat4(&XMFLOAT4(_eye.x, _eye.y, _eye.z, 1.0f)), moveAmount);
    XMStoreFloat3(&_eye, newPosition);
    //rebuildViewMatrix();
  }
}

void Camera::strafe(const float distance, const Terrain& terrain) {
  if (moveable) {
    XMVECTOR moveAmount = XMVectorScale(XMLoadFloat4(&XMFLOAT4(right.x, right.y, right.z, 0.0f)), distance);
    XMVECTOR newPosition = XMVectorAdd(XMLoadFloat4(&XMFLOAT4(_eye.x, _eye.y, _eye.z, 1.0f)), moveAmount);
    XMStoreFloat3(&_eye, newPosition); 
    if (!freeFlying) {
      float height = terrain.getCameraHeight(_eye.x, _eye.z) - 82.0f;
      _eye.y = height + 2.0f;
      //_at.y += height;
    }
    //rebuildViewMatrix();
  }
}

void Camera::rotatePitch(const float angle) {
  if (moveable) {
    XMMATRIX rotationMatrix = XMMatrixRotationAxis(XMLoadFloat4(&XMFLOAT4(right.x, right.y, right.z, 0.0f)), angle);
    XMVECTOR newLookVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(_at.x, _at.y, _at.z, 0.0f)), rotationMatrix);
    XMVECTOR newUpVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(up.x, up.y, up.z, 0.0f)), rotationMatrix);
    XMStoreFloat3(&_at, XMVector4Normalize(newLookVector));
    if (freeFlying) {
      XMStoreFloat3(&up, XMVector4Normalize(newUpVector));
    }
    //rebuildViewMatrix();
  }
}

void Camera::rotateYaw(const float angle) {
  if (moveable) {
    XMMATRIX rotationMatrix = XMMatrixRotationY(angle);
    XMVECTOR newLookVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(_at.x, _at.y, _at.z, 0.0f)), rotationMatrix);
    XMVECTOR newUpVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(up.x, up.y, up.z, 0.0f)), rotationMatrix);
    XMVECTOR newRightVector = XMVector4Transform(XMLoadFloat4(&XMFLOAT4(right.x, right.y, right.z, 0.0f)), rotationMatrix);
    XMStoreFloat3(&_at, XMVector4Normalize(newLookVector));
    if (freeFlying) {
      XMStoreFloat3(&up, XMVector4Normalize(newUpVector));
    }
    XMStoreFloat3(&right, XMVector4Normalize(newRightVector));
  }
}