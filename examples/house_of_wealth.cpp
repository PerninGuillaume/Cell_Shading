#include "house_of_wealth.h"

#include <iostream>
#include <memory>

#include "../Camera.h"
#include "../callback.h"
#include "../Helper.h"
#include "../Model.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace house_of_wealth {

bool use_im_gui = true;
float lastFrame = 0.0f;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));


program *init_program(GLFWwindow *window, const std::string& vertex_shader_filename,
                      const std::string& fragment_shader_filename, const std::string& geometry_shader_filename = "") {
  program *program = program::make_program(vertex_shader_filename,
                                           fragment_shader_filename, geometry_shader_filename);
  std::cout << program->get_log();
  if (!program->is_ready()) {
    throw "Program is not ready";
  }
  program->use();
  return program;
}

void display(GLFWwindow *window) {

  //Capture the mouse
  if (use_im_gui) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
  }
  else
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // Setup Platfrom/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 450");
  ImGui::StyleColorsDark();

  program *shader_house= init_program(window, "shaders/vertex_model.glsl",
                                                            "shaders/fragment_model.glsl");
  //stbi_set_flip_vertically_on_load(true);
  Model house_of_wealth("models/Auction House/model/model1.obj");

  glEnable(GL_DEPTH_TEST);

  //Setup of different default values
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon = true;
  bool no_texture = false;
  bool display_normals = false;
  bool flat_look = false;
  float light_ambient = 0.7f;
  float light_diffuse = 0.8f;
  float light_dir[3] = {-0.3f, -0.7f, -0.3f};
  ImVec4 some_color = ImVec4(0.45f, 0.55f, 0.6f, 1.00f);
  float alpha_clip = 0.3f;
  float offset = 0.0f;
  Helper helper = Helper(camera, use_im_gui);

  while (!glfwWindowShouldClose(window)) {

    if (use_im_gui) {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
    }

    float currentFrame = glfwGetTime();
    helper.deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwSetWindowUserPointer(window, &helper);
    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera->fov_camera), 800.0f / 600.0f,
                                            0.1f, 1000.0f);
    glm::mat4 view = camera->view_matrix();



    //--------------------House of Wealth rendering-----------------------

    if (wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    view = camera->view_matrix();

    shader_house->set_uniform_mat4("view", view);
    shader_house->set_uniform_mat4("projection", projection);
    shader_house->set_uniform_float("alpha_clip", alpha_clip);
    shader_house->set_uniform_bool("use_zAtoon", use_zAtoon);
    shader_house->set_uniform_bool("no_texture", no_texture);


    glm::mat4 model = glm::mat4(1.0f);
    //model = glm::translate(model, glm::vec3(1.0f, -10.0f, -25.0f));
    model = glm::scale(model, glm::vec3(8.0f));
    shader_house->set_uniform_mat4("model", model);

    house_of_wealth.draw(shader_house);





    if (use_im_gui) {
      ImGui::Begin("Windfall options");
      ImGui::Checkbox("WireFrame", &wireframe);
      ImGui::Checkbox("Use Zatoon", &use_zAtoon);
      ImGui::Checkbox("No texture", &no_texture);
      ImGui::Checkbox("Enable lighting", &with_lighting);
      ImGui::Checkbox("Display Normals", &display_normals);
      ImGui::Checkbox("Flat look", &flat_look);
      ImGui::SliderFloat("Alpha clip", &alpha_clip, 0.0f, 1.0f);
      ImGui::SliderFloat("Light diffuse", &light_diffuse, 0.0f, 1.0f);
      ImGui::SliderFloat("Light ambient", &light_ambient, 0.0f, 1.0f);
      ImGui::ColorEdit3("Some color", (float*)&some_color);
      ImGui::SliderFloat3("Light direction", light_dir, -1.0f, 1.0f);
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();

      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    glfwSwapBuffers(window);
    glfwPollEvents();

  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

}
}

