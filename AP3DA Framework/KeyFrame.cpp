#include "KeyFrame.h"
#include <sstream>

template <class T>
T convertStringToNumber(std::string toConvert) {
  T r;
  std::stringstream(toConvert) >> r;
  return r;
}

void KeyFrame::loadFrame(xml_node<>* frameNode) {
  int bone;
  XMFLOAT3 rotation, position;

  numFrames = convertStringToNumber<int>(frameNode->first_attribute("num_frames")->value());
  for (xml_node<>* boneNode = frameNode->first_node(); boneNode; boneNode = boneNode->next_sibling()) {
    bone = convertStringToNumber<int>(boneNode->first_attribute("id")->value());

    if (boneNode->first_node("position")) {
      xml_node<>* posNode = boneNode->first_node("position");
      position.x = convertStringToNumber<float>(posNode->first_attribute("x")->value());
      position.y = convertStringToNumber<float>(posNode->first_attribute("y")->value());
      position.z = convertStringToNumber<float>(posNode->first_attribute("z")->value());
      updatePositions.push_back(true);
    }
    else {
      updatePositions.push_back(false);
    }

    if (boneNode->first_node("rotation")) {
      xml_node<>* rotNode = boneNode->first_node("rotation");
      rotation.x = convertStringToNumber<float>(rotNode->first_attribute("x")->value());
      rotation.y = convertStringToNumber<float>(rotNode->first_attribute("y")->value());
      rotation.z = convertStringToNumber<float>(rotNode->first_attribute("z")->value());
      updateRotations.push_back(true);
    }
    else {
      updateRotations.push_back(false);
    }

    bones.push_back(bone);
    positions.push_back(position);
    rotations.push_back(rotation);
  }
}