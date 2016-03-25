#pragma once
#include "GameObject.h"
#include "KeyFrame.h"
#include "Terrain.h"
#include <vector>

class Skeleton {
public:
  Skeleton(int numBones, ID3D11Device* d3dDevice, ID3D11DeviceContext* pImmediateContext);
  ~Skeleton();

  inline void addBone(GameObject* bone) { bones.push_back(bone); }
  void setTerrain(Terrain* const terrain) { this->terrain = terrain; }
  void setAnimation(const std::vector<KeyFrame>& animation) { this->animation = animation; }
  void addWaypoint(XMFLOAT3 waypoint) { waypoints.push_back(waypoint); }
  void setRoot(GameObject* const rootNode) { this->root = rootNode; rootInitialPosition = root->GetPosition(); }
  void runAnimation(const std::vector<KeyFrame>& frames);
  
  void walk();
  void update(float t);
  void draw(ConstantBuffer& cb, ID3D11Buffer* constantBuffer);
private:
  bool updateInstanceBuffer;
  int currentFrame = 0;
  int frameCount = 1;
  int target = 0;
  int move = 1;
  std::vector<InstanceData> instanceData;
  std::vector<XMFLOAT3> initialPositions, initialRotations;
  std::vector<GameObject* const> bones;
  std::vector<KeyFrame> animation;
  std::vector<XMFLOAT3> waypoints;
  Terrain* terrain;
  GameObject* root;
  XMFLOAT3 rootInitialPosition, facing;
  float previousRotation = 0.0f;

  ID3D11Buffer* instanceBuffer;
  ID3D11DeviceContext* pImmediateContext;
  ID3D11Device* d3dDevice;
};