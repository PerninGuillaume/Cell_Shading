#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "program.h"
#include "Camera.h"
#include "Helper.h"

class Display_template {
 public:
  Display_template(GLFWwindow *window, const std::string& path, bool use_im_gui);
 private:

  void display(const std::string& path);
  GLFWwindow* window;
  std::shared_ptr<Camera> camera;
  bool use_im_gui = false;
  float lastFrame = 0.0f;
};