#include <string>
#include <iostream>
#include "Display_template.h"
#include "Model.h"
#include "callback.h"

Display_template::Display_template(GLFWwindow *window, const std::string &path, bool use_im_gui)
: window(window)
, use_im_gui(use_im_gui)
{
  camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  display(path);
}

program *init_program(const std::string& vertex_shader_filename,
                      const std::string& fragment_shader_filename) {
  program *program = program::make_program(vertex_shader_filename,
                                           fragment_shader_filename);
  std::cout << program->get_log();
  if (!program->is_ready()) {
    throw "Program is not ready";
  }
  program->use();
  return program;
}


void Display_template::display(const std::string& path) {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  program *program = init_program("shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");

  Model model_mesh(path);

  glEnable(GL_DEPTH_TEST);
  //Capture the mouse
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  Helper helper = Helper(camera, use_im_gui);
  while (!glfwWindowShouldClose(window)) {

    float currentFrame = glfwGetTime();
    helper.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    glfwSetWindowUserPointer(window, &helper);

    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera->fov_camera), 800.0f / 600.0f,
                                            0.01f, 1000.0f);
    glm::mat4 view = camera->view_matrix();

    program->set_uniform_mat4("view", view);
    program->set_uniform_mat4("projection", projection);

    //program->set_uniform_vec3("viewPosition", camera.position);
    glm::mat4 model = glm::mat4(1.0f);
    program->set_uniform_mat4("model", model);

    model_mesh.draw(program);

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  glfwTerminate();

}

