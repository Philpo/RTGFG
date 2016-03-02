#include "Voxel.h"

Voxel::Voxel(Geometry geometry, Material material) : GameObject("voxel", geometry, material), visible(true) {
}

Voxel::~Voxel() {
}