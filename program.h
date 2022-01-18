#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <map>

class program {
 public:
  program();
  ~program();
  static program *make_program(const std::string &vertex_shader_filename, const std::string& fragment_shader_filenames,
                               const std::string &geometry_shader_filename = "", const std::map<std::string, std::string>& values_to_replace = {});
  static program *make_program(const std::string &compute_shader_filename, const std::map<std::string, std::string>& values_to_replace = {});
  static program *make_program_multiple_files(const std::string &vertex_shader_filename, const std::vector<std::string>& fragment_shader_filenames,
                               const std::string &geometry_shader_filename = "", const std::map<std::string, std::string>& values_to_replace = {});

  std::string get_log();
  bool is_ready();
  void use();
  void set_uniform_float(const std::string& var_name, float value);
  void set_uniform_vec3(const std::string& var_name, float x, float y, float z);
  void set_uniform_vec3(const std::string& var_name, const glm::vec3& vec3);
  void set_uniform_vec3(const std::string &var_name, float val);
  void set_uniform_vec4(const std::string &var_name, const glm::vec4& vec4);
  void set_uniform_int(const std::string &var_name, int value);
  void set_uniform_mat4(const std::string &var_name, const glm::mat4& mat);
  void set_uniform_vector_mat4(const std::string& var_name, const std::vector<glm::mat4>& data);
  void set_uniform_bool(const std::string &var_name, bool value);
  void set_uniform_vector_float(const std::string &var_name, size_t count, float *pointer);
  void set_uniform_vector_float(const std::string &var_name, const std::vector<float>& vec);
  void set_uniform_vector_vec3(const std::string& var_name, const std::vector<glm::vec3>& vec);
  void set_uniform_vector_vec3(const std::string& var_name, size_t size, void* data);
  void set_uniform_vector_bool(const std::string& var_name, const std::vector<GLint>& vec);
  void set_uniform_vector_int(const std::string& var_name, size_t size, void* data);
  void set_uniform_vector_bool(const std::string& var_name, size_t size, void* data);

  GLuint my_program_ = 0;
  std::vector<GLint> shaders_;

};