#include "windfall.h"

#include <iostream>
#include <memory>

#include "../Camera.h"
#include "../callback.h"
#include "../Skybox.h"
#include "../Helper.h"
#include "../Model.h"
#include "../misc.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

namespace windfall {

bool use_im_gui = true;
float lastFrame = 0.0f;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));

unsigned int water_create_VAO() {
  float heightf = -11.0;
  float waterVertices[] = {
      -20000.0f, heightf, -20000.0f, 20000.0f, heightf, -20000.0f, 20000.0f, heightf, 20000.0f, 20000.0f, heightf, 20000.0f, -20000.0f, heightf, 20000.0f, -20000.0f, heightf, -20000.0f,};

  unsigned int waterVAO, waterVBO;
  glGenVertexArrays(1, &waterVAO);
  glGenBuffers(1, &waterVBO);
  glBindVertexArray(waterVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(waterVertices), &waterVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

  return waterVAO;
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

  program *shader_normals = init_program("shaders/vertex_normals.glsl", "shaders/fragment_normals.glsl",
                                         "shaders/geometry_normals.glsl");
  program *program_windfall;
  program *program_windfall_without_lighting = init_program("shaders/vertex_model.glsl",
                                  "shaders/fragment_model.glsl");
  program *program_windfall_with_lighting = init_program("shaders/vertex_windfall.glsl",
                                           "shaders/fragment_windfall.glsl");
  program_windfall = program_windfall_with_lighting;
  program *program_skybox = init_program("shaders/vertex_skybox.glsl",
                                         "shaders/fragment_skybox.glsl");
  program *program_water = init_program("shaders/vertex_water.glsl",
                                        "shaders/fragment_water.glsl");
  program *program_clouds = init_program("shaders/vertex_clouds.glsl",
                                         "shaders/fragment_clouds.glsl");
  //stbi_set_flip_vertically_on_load(true);
  Model windfall_flat("models/Windfall Island/Windfall/Windfall_save.obj");
  Model windfall_smooth("models/Windfall Island/Windfall/Windfall.obj");

  glEnable(GL_DEPTH_TEST);

  //Skybox

  unsigned int cubemapTexture = loadSkyBox(program_skybox);
  unsigned int skyboxVAO = skyBox_create_VAO();

  // Water

  unsigned int waterVAO = water_create_VAO();
  set_zAtoon(program_windfall);


  //Setup of different default values
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon = false;
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


  // Clouds
  std::vector<unsigned int> cloudsTextures = loadClouds();
  unsigned int cloudsVAO = clouds_create_VAO();
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


    //------------------ Water rendering-----------------------------

    program_water->use();
    glm::mat4 view = camera->view_matrix();
    glm::mat4 model = glm::mat4(1.0f);
    program_water->set_uniform_mat4("view", view);
    program_water->set_uniform_mat4("projection", projection);
    program_water->set_uniform_mat4("model", model);

    glBindVertexArray(waterVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);


    //--------------------Windfall rendering-----------------------

    if (wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (with_lighting) {
      program_windfall = program_windfall_with_lighting;
    } else {
      program_windfall = program_windfall_without_lighting;
    }
    view = camera->view_matrix();

    program_windfall->set_uniform_mat4("view", view);
    program_windfall->set_uniform_mat4("projection", projection);
    program_windfall->set_uniform_float("alpha_clip", alpha_clip);
    program_windfall->set_uniform_bool("use_zAtoon", use_zAtoon);
    program_windfall->set_uniform_bool("no_texture", no_texture);

    //program_windfall->set_uniform_vec3("dirLight.direction", -0.3f, -0.7f, -0.3f);
    program_windfall->set_uniform_vec3("dirLight.direction", light_dir[0], light_dir[1], light_dir[2]);

    program_windfall->set_uniform_vec3("dirLight.ambient",  light_ambient);
    program_windfall->set_uniform_vec3("dirLight.diffuse", light_diffuse);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0f, -10.0f, -25.0f));
    model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
    program_windfall->set_uniform_mat4("model", model);

    if (flat_look)
      windfall_flat.draw(program_windfall);
    else
      windfall_smooth.draw(program_windfall);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);




//-----------------------Normals------------------------------------
    if (display_normals) {
      shader_normals->set_uniform_mat4("projection", projection);
      shader_normals->set_uniform_mat4("view", view);
      shader_normals->set_uniform_mat4("model", model);
      if (flat_look)
        windfall_flat.draw(shader_normals);
      else
        windfall_smooth.draw(shader_normals);
    }

//--------------------------Skybox rendering---------------------------
    program_skybox->use();

    glDepthFunc(GL_LEQUAL);
    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
    program_skybox->set_uniform_mat4("view", view);
    program_skybox->set_uniform_mat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    //------------------ Clouds rendering --------------------------

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    program_clouds->use();

    program_clouds->set_uniform_int("tex_cloud", 0);
    program_clouds->set_uniform_int("tex_cloud_mask", 1);
    view = glm::mat4(glm::mat3(camera->view_matrix())); // Remove the translation from the view matrix
//    view = camera->view_matrix();
    if (offset > 1.0)
      offset = -1.0;
    program_clouds->set_uniform_float("offset", offset);

    model = glm::mat4(1.0f);
//    view = glm::translate(view, glm::vec3(-offset * 10));
    program_clouds->set_uniform_mat4("view", view);
    program_clouds->set_uniform_mat4("projection", projection);
    program_clouds->set_uniform_float("alpha_clip", alpha_clip);
    program_clouds->set_uniform_mat4("model", model);
    program_clouds->set_uniform_vec3("camera_right", camera->right);
    program_clouds->set_uniform_vec3("camera_up", camera->up);
    offset += helper.deltaTime / 100;
//    std::cout <<


    glBindVertexArray(cloudsVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[2]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, cloudsTextures[3]);
    glDrawArrays(GL_TRIANGLES, 0, 6 * 4);
    glBindVertexArray(0);

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

