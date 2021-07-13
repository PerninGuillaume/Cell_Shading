#include "house_of_wealth.h"

#include <iostream>
#include <memory>

#include "../Camera.h"
#include "../callback.h"
#include "../Helper.h"
#include "../Model.h"
#include "../misc.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "../Shadow.h"

namespace house_of_wealth {

bool use_im_gui = true;
float lastFrame = 0.0f;

std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));


void display(GLFWwindow *window) {

  int width, height;
  glfwGetWindowSize(window, &width, &height);
  unsigned int SRC_WIDTH = width;
  unsigned int SRC_HEIGHT = height;
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

  program *shadow_shader_depth = init_program("shaders/vertex_shadow_depth.glsl",
                                              "shaders/fragment_shadow_depth.glsl");
  program *shader_character = init_program("shaders/vertex_link.glsl",
                                 "shaders/fragment_link.glsl");
  program *shader_house_no_light = init_program("shaders/vertex_model.glsl",
                                                            "shaders/fragment_model.glsl");
  program *shader_house_with_light = init_program("shaders/vertex_link.glsl",
                                      "shaders/fragment_link.glsl");
  program *quad_depth_shader = init_program("shaders/vertex_normalized_coord.glsl", "shaders/fragment_quad_depth.glsl");
  program *shader_house = shader_house_with_light;

  Model house_of_wealth("models/Auction House/model/model1.obj");
  Model house_of_wealth_smooth("models/Auction House/model/model1_smooth.obj");
  Model link("models/link-cartoon/source/LinkCartoon.fbx");
  Model ganondorf("models/Ganondorf Figurine/133.obj");

  unsigned int size_shadow_texture = 4096;
  Shadow shadow = Shadow(size_shadow_texture, size_shadow_texture);

  glEnable(GL_DEPTH_TEST);

  set_zAtoon(shader_character);

  //Setup of different default values
  bool with_lighting = true;
  bool wireframe = false;
  bool use_zAtoon_house = false;
  bool use_zAtoon_character = true;
  bool use_shadow = true;
  bool no_texture = false;
  bool display_normals = false;
  bool flat_look = false;
  bool peter_paning = false;
  bool pcf = false;
  float shadow_bias = 0.005f;
  float near_plane_light = 1.0f, far_plane_light = 17.5f;
  float light_ambient = 0.7f;
  float light_diffuse = 0.8f;
  float light_dir[3] = {-0.3f, -0.7f, -0.3f};
  float light_pos[3] = {-2.0f, 4.0f, 8.681f}; //need a position for shadow
  float link_translation[3] = {0.0f, -0.36f, -3.571f};
  float ganon_translation[3] = {3.55f, -2.0f, -1.2f};
  ImVec4 some_color = ImVec4(0.45f, 0.55f, 0.6f, 1.00f);
  float alpha_clip = 0.3f;
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

    glfwSetWindowUserPointer(window, &helper); //For callbacks, set the pointer to use
    processInput(window); //input

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Options of rendering
    if (wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (with_lighting)
      shader_house = shader_house_with_light;
    else
      shader_house = shader_house_no_light;

    glm::mat4 projection = glm::perspective(glm::radians(camera->fov_camera), 800.0f / 600.0f,
                                            0.1f, 1000.0f);
    glm::mat4 view = camera->view_matrix();

// 1. Render depth of scene to texture (from light's perspective)
    // --------------------------------------------------------------
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane_light, far_plane_light);
    lightView = glm::lookAt(glm::vec3(light_pos[0], light_pos[1], light_pos[2]), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;


    glViewport(0, 0, shadow.shadow_width, shadow.shadow_height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //Render

    //Render link depth to a texture
    glm::mat4 model_mat_link = glm::mat4(1.0f);
    model_mat_link = glm::translate(model_mat_link, glm::vec3(link_translation[0], link_translation[1], link_translation[2]));
    model_mat_link = glm::rotate(model_mat_link, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model_mat_link = glm::scale(model_mat_link, glm::vec3(0.5));

    shadow_shader_depth->set_uniform_mat4("model", model_mat_link);
    shadow_shader_depth->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);

    if (peter_paning)
      glCullFace(GL_FRONT);
    link.draw(shadow_shader_depth);

    //Render Ganondorf depth on the same texture
    glm::mat4 model_mat_ganon = glm::mat4(1.0f);
    model_mat_ganon = glm::translate(model_mat_ganon, glm::vec3(ganon_translation[0], ganon_translation[1], ganon_translation[2]));
    model_mat_ganon = glm::scale(model_mat_ganon, glm::vec3(4.0f));
    shadow_shader_depth->set_uniform_mat4("model", model_mat_ganon);

    ganondorf.draw(shadow_shader_depth);
    glCullFace(GL_BACK);//In all cases we cull the face normally (peter paning or not)

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // reset viewport

    glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    std::vector<program*> shaders = {shader_house, shader_character};
    for (const auto& shader : shaders) {
      shader->set_uniform_mat4("view", view);
      shader->set_uniform_mat4("projection", projection);
      shader->set_uniform_float("alpha_clip", alpha_clip);
      shader->set_uniform_bool("no_texture", no_texture);
      shader->set_uniform_bool("pcf", pcf);
      shader->set_uniform_bool("use_shadow", use_shadow);
      shader->set_uniform_float("shadow_bias", shadow_bias);
      shader->set_uniform_vec3("dirLight.direction", light_dir[0], light_dir[1], light_dir[2]);
      shader->set_uniform_vec3("dirLight.ambient",  light_ambient);
      shader->set_uniform_vec3("dirLight.diffuse", light_diffuse);

    }

    // 2. render scene as normal using the generated depth/shadow map
    // --------------------------------------------------------------
    
    //--------------------House of Wealth rendering-----------------------


    shader_house->set_uniform_bool("use_zAtoon", use_zAtoon_house);

    glm::mat4 model_house = glm::mat4(1.0f);
    model_house = glm::translate(model_house, glm::vec3(0.0f, -2.0f, -2.0f));
    model_house = glm::scale(model_house, glm::vec3(8.0f));
    shader_house->set_uniform_mat4("model", model_house);

    shader_house->set_uniform_int("shadowMap", 1);
    shader_house->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    if (flat_look)
      house_of_wealth.draw(shader_house);
    else
      house_of_wealth_smooth.draw(shader_house);


    //----------------------Link-----------------------------------------


    shader_character->set_uniform_mat4("model", model_mat_link);
    shader_character->set_uniform_bool("use_zAtoon", use_zAtoon_character);

    shader_character->set_uniform_int("shadowMap", 0);
    shader_character->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    link.draw(shader_character);


    //----------------------Ganondorf-----------------------------------------
    shader_character->set_uniform_mat4("model", model_mat_ganon);
    //TODO why is the triforce on his hand weird

    shader_character->set_uniform_int("shadowMap", 1);
    shader_character->set_uniform_mat4("lightSpaceMatrix", lightSpaceMatrix);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);

    ganondorf.draw(shader_character);


    quad_depth_shader->use();
    quad_depth_shader->set_uniform_int("depthMap", 0);
    quad_depth_shader->set_uniform_float("near_plane", near_plane_light);
    quad_depth_shader->set_uniform_float("far_plane", far_plane_light);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depthMapTexture);
    //renderQuad();

    if (use_im_gui) {
      ImGui::Begin("House of Wealth options");
      ImGui::Checkbox("Shadow", &use_shadow);
      ImGui::Checkbox("Peter Paning", &peter_paning);
      ImGui::Checkbox("PCF", &pcf);
      ImGui::SliderFloat("Shadow bias", &shadow_bias, 0.0f, 0.1f);
      ImGui::SliderFloat("Near plane light frustrum", &near_plane_light, 0.0f, 2.0f);
      ImGui::SliderFloat("Far plane light frustrum", &far_plane_light, 10.0f, 100.0f);
      ImGui::Checkbox("WireFrame", &wireframe);
      ImGui::Checkbox("Use Zatoon for character", &use_zAtoon_character);
      ImGui::Checkbox("Use Zatoon for the House", &use_zAtoon_house);
      ImGui::Checkbox("No texture", &no_texture);
      ImGui::Checkbox("Enable lighting", &with_lighting);
      ImGui::Checkbox("Display Normals", &display_normals);
      ImGui::Checkbox("Flat look", &flat_look);
      ImGui::SliderFloat("Alpha clip", &alpha_clip, 0.0f, 1.0f);
      ImGui::SliderFloat("Light diffuse", &light_diffuse, 0.0f, 1.0f);
      ImGui::SliderFloat("Light ambient", &light_ambient, 0.0f, 1.0f);
      ImGui::SliderFloat3("Light position", light_pos, -10.0f, 10.0f);
      ImGui::ColorEdit3("Some color", (float*)&some_color);
      ImGui::SliderFloat3("Light direction", light_dir, -1.0f, 1.0f);
      ImGui::SliderFloat3("Link translation", link_translation, -10.0f, 10.0f);
      ImGui::SliderFloat3("Ganondorf translation", ganon_translation, -10.0f, 10.0f);

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

