#include "main.h"
#include "program.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

unsigned int SRC_WIDTH = 800;
unsigned int SRC_HEIGHT = 600;

void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

bool initGlew() {
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cout << glewGetErrorString(err);
    return 1;
  }
  return 0;
}

bool init_gl();


GLFWwindow* init_glfw() {
  bool glfw_init = glfwInit();
  if (!glfw_init) {
    std::cout << "Failed to init glfw" << std::endl;
    glfwTerminate();
    return nullptr;
  }
  glfwSetErrorCallback(error_callback);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window." << std::endl;
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  return window;
}

int main() {
  GLFWwindow* window = init_glfw();
  if (window == nullptr || initGlew() == 1)
    return 1;

  try {
    //texture::display(window);
    //phong::display(window);
    //gouraud::display(window);
    //coordinates::display(window);
    //camera::display_camera(window);
    //colors::display(window);
    //light_map::display(window);
    //multiples_lights::display(window);
    //backpack::display(window);
    //link::display(window);
    //nordic_village::display(window);
    //cavern::display(window);
    //isometric_world::display(window);
    bomb_shop::display(window);
  } catch (const char* msg) {
    std::cerr << msg << std::endl;
  }
  return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  if (!window)
    std::cout << "Window not given";
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}
