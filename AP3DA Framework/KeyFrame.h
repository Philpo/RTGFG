#pragma once
#include "RapidXML\rapidxml.hpp"
#include <DirectXMath.h>
#include <vector>

using namespace rapidxml;
using namespace DirectX;

class KeyFrame {
public:
  KeyFrame() {}
  ~KeyFrame() {}

  inline const std::vector<int>& getTargets() const { return bones; }
  inline int getNumFrames() const { return numFrames; }
  inline const std::vector<bool>& getUpdateRotations() const { return updateRotations; }
  inline const std::vector<bool>& getUpdatePositions() const { return updatePositions; }
  inline const std::vector<XMFLOAT3>& getRotations() const { return rotations; }
  inline const std::vector<XMFLOAT3>& getPositions() const { return positions; }

  void loadFrame(xml_node<>* frameNode);
private:
  int numFrames;
  std::vector<int> bones;
  std::vector<bool> updateRotations, updatePositions;
  std::vector<XMFLOAT3> rotations, positions;
};