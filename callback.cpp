#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Helper.h"

Helper* get_helper(GLFWwindow* window) {
  void* user_pointer = glfwGetWindowUserPointer(window);
  if (user_pointer == nullptr) {
    throw "user pointer not set";
  }
  auto* helper = static_cast<Helper *>(user_pointer);
  return helper;
}

void processInput(GLFWwindow *window) {
  Helper* helper = get_helper(window);
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    helper->camera->shift_pressed(true);
  else
    helper->camera->shift_pressed(false);

  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    helper->camera->ctrl_pressed(true);
  else
    helper->camera->ctrl_pressed(false);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    helper->camera->processKeyboard(Camera_Movement::FORWARD, helper->deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    helper->camera->processKeyboard(Camera_Movement::BACKWARD, helper->deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    helper->camera->processKeyboard(Camera_Movement::LEFT, helper->deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    helper->camera->processKeyboard(Camera_Movement::RIGHT, helper->deltaTime);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  Helper* helper = get_helper(window);

  if (helper->firstMouse) {
    helper->lastXMouse = xpos;
    helper->lastYMouse = ypos;
    helper->firstMouse = false;
  }

  double xoffset = xpos - helper->lastXMouse;
  double yoffset = helper->lastYMouse - ypos;
  helper->lastXMouse = xpos;
  helper->lastYMouse = ypos;

  if (!helper->right_button_mouse_clicked && helper->use_im_gui)
    return;
  helper->camera->processMouse(xoffset, yoffset);
}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  Helper* helper = get_helper(window);
  helper->camera->processScroll(yoffset);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  Helper* helper = get_helper(window);
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    helper->right_button_mouse_clicked = true;
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    helper->right_button_mouse_clicked = false;
  }
}