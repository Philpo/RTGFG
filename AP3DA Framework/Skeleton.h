#pragma once
#include "GameObject.h"
#include "KeyFrame.h"
#include <vector>

class Skeleton {
public:
  Skeleton(int numBones, ID3D11Device* d3dDevice, ID3D11DeviceContext* pImmediateContext);
  ~Skeleton();

  inline void addBone(GameObject* bone) { bones.push_back(bone); }
  void runAnimation(const std::vector<KeyFrame>& frames);

  void update(float t);
  void draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer);
private:
  bool updateInstanceBuffer;
  int currentFrame = 0;
  int frameCount = 1;
  std::vector<InstanceData> instanceData;
  std::vector<XMFLOAT3> initialPositions, initialRotations;
  std::vector<GameObject* const> bones;

  ID3D11Buffer* instanceBuffer;
  ID3D11DeviceContext* pImmediateContext;
  ID3D11Device* d3dDevice;
};