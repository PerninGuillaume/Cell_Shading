#include "program.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

program::program()
{}

program::~program() {
  glDeleteProgram(this->my_program_);
  for (GLint shader : shaders_) {
    glDeleteShader(shader);
  }
}

std::string read_file(std::string filename) {
  std::ifstream ifstream(filename);
  std::stringstream buffer;
  buffer << ifstream.rdbuf();
  std::string str = buffer.str();
  return str;
}

program *program::make_program(const std::string &vertex_shader_filename, const std::string &fragment_shader_filename) {
  const std::string vertex_shader_src = read_file(vertex_shader_filename);
  const std::string fragment_shader_src = read_file(fragment_shader_filename);
  if (vertex_shader_src.empty())
    std::cout << vertex_shader_filename << " is read to be empty !\n";
  if (fragment_shader_src.empty())
    std::cout << fragment_shader_filename << " is read to be empty !\n";
  program* program_res = new program();
  GLuint my_program = glCreateProgram();

  GLuint my_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  program_res->shaders_.emplace_back(my_fragment_shader);
  const GLchar *fragment_src = (const GLchar *)fragment_shader_src.c_str();
  glShaderSource(my_fragment_shader, 1, &fragment_src, 0);
  glCompileShader(my_fragment_shader);

  GLuint my_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  program_res->shaders_.emplace_back(my_vertex_shader);
  const GLchar *vertex_src = (const GLchar *)vertex_shader_src.c_str();
  glShaderSource(my_vertex_shader, 1, &vertex_src, 0);
  glCompileShader(my_vertex_shader);

  glAttachShader(my_program, my_fragment_shader);
  glAttachShader(my_program, my_vertex_shader);
  glLinkProgram(my_program);

  program_res->my_program_ = my_program;
  return program_res;

}

std::string program::get_log() {
  std::string log_final = "";
  for (GLint shader : this->shaders_) {
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      GLint maxLength = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
      char *log = new char[maxLength];
      glGetShaderInfoLog(shader, maxLength, &maxLength, log);
      log_final.append(log);
      log_final += '\n';
    }
  }
  int success;
  glGetProgramiv(this->my_program_, GL_LINK_STATUS, &success);
  if (!success) {
    GLint length_log;
    glGetProgramiv(this->my_program_, GL_INFO_LOG_LENGTH, &length_log);

    char *log = new char[length_log];
    glGetProgramInfoLog(this->my_program_, length_log, NULL, log);
    log_final.append(log);
  }

  return log_final;
}

bool program::is_ready() {
  for (GLint shader : this->shaders_) {
    GLint is_compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {
      return false;
    }
  }

  GLint statusLink;
  glGetProgramiv(this->my_program_, GL_LINK_STATUS, &statusLink);
  return statusLink == GL_TRUE;
}

void program::use() {
  glUseProgram(this->my_program_);
}

void program::set_uniform_float(const std::string& var_name, float value) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1f(uniform_location, value);
}

void program::set_uniform_int(const std::string& var_name, int value) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1i(uniform_location, value);
}

void program::set_uniform_vec3(const std::string &var_name, float x, float y, float z) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform3f(uniform_location, x, y, z);
}

void program::set_uniform_vec3(const std::string &var_name, const glm::vec3 &vec3) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform3f(uniform_location, vec3.x, vec3.y, vec3.z);
}

void program::set_uniform_vec3(const std::string &var_name, float val) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform3f(uniform_location, val, val, val);
}

void program::set_uniform_mat4(const std::string &var_name, const glm::mat4 &mat) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(mat));
}


