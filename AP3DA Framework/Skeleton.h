#pragma once
#include "GameObject.h"
class Skeleton : public GameObject {
public:
  Skeleton(string type, Geometry geometry, Material material);
  ~Skeleton();
};