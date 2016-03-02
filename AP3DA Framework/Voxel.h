#pragma once
#include "GameObject.h"
class Voxel : public GameObject {
public:
  Voxel(Geometry geometry, Material material);
  ~Voxel();

  inline bool isVisible() const { return visible; }
  inline void setVisible(bool visible) { this->visible = visible; }
private:
  bool visible;
};

