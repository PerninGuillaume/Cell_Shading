#pragma once
#include <memory>
#include "Camera.h"

//This is a class to store info to pass to glfw pointer
//This is done so because we can't pass custom argument to pointers
class Helper {
 public:
  Helper(std::shared_ptr<Camera> camera, bool use_im_gui);
  std::shared_ptr<Camera> camera;
  float deltaTime = 0.0f;
  bool firstMouse = true;
  float lastXMouse = 800 / 2;
  float lastYMouse = 600 / 2;
  bool use_im_gui = true;
  bool right_button_mouse_clicked = false;
};
