#include "main.h"
#include "program.h"
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

unsigned int SRC_WIDTH = 1920;
unsigned int SRC_HEIGHT = 1080;

void error_callback(int, const char* description)
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

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
           type, severity, message );
}

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
  glfwWindowHint(GLFW_SAMPLES, 4);
  GLFWwindow* window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "The legend of POGL", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window." << std::endl;
    glfwTerminate();
    return nullptr;
  }
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  std::cout << "Size of window created : " << width << ", " << height << std::endl;
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  return window;
}

int main() {
  GLFWwindow* window = init_glfw();
  if (window == nullptr || initGlew() == 1)
    return 1;
  // During init, enable debug output
  //glEnable              ( GL_DEBUG_OUTPUT );
  //glDebugMessageCallback( MessageCallback, 0 );

  try {
    //link::display(window);
    //Display_template ganondorf(window, "models/Ganondorf Figurine/133.obj", false);
    windfall::display(window, false, true);
    //house_of_wealth::display(window, true);
  } catch (const char* msg) {
    std::cerr << msg << std::endl;
  }
  return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  std::cout << "width : " << width << ", height : " << height << std::endl;
  if (!window)
    std::cout << "Window not given";
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}
