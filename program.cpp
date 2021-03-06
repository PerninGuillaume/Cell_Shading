#include "program.h"
#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>


GLenum glCheckError_(const char *file, int line)
{
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR)
  {
    std::string error;
    switch (errorCode)
    {
      case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
      case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
      case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
      case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
      case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
      case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
    }
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
  }
  return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


program::program()
{}

program::~program() {
  glDeleteProgram(this->my_program_);
  for (GLint shader : shaders_) {
    glDeleteShader(shader);
  }
}

std::string read_file(std::string filename, const std::map<std::string, std::string>& values_to_replace) {
  std::ifstream ifstream(filename);
  std::stringstream buffer;
  buffer << ifstream.rdbuf();
  std::string str = buffer.str();

  for (const auto& value_to_replace : values_to_replace) {
    auto pos = str.find(value_to_replace.first);
    if (pos != std::string::npos)
      str.replace(pos, value_to_replace.first.length(), value_to_replace.second);
  }

  return str;
}

program *program::make_program_multiple_files(const std::string &vertex_shader_filename, const std::vector<std::string>& fragment_shader_filenames,
                               const std::string &geometry_shader_filename, const std::map<std::string, std::string>& values_to_replace) {
  const std::string vertex_shader_src = read_file(vertex_shader_filename, values_to_replace);
  if (vertex_shader_src.empty())
    std::cout << vertex_shader_filename << " is read to be empty !\n";

  std::vector<std::string> fragment_shader_srcs;
  for (const auto &fragment_shader_filename : fragment_shader_filenames) {
    const std::string fragment_shader_src = read_file(fragment_shader_filename, values_to_replace);
    if (fragment_shader_src.empty())
      std::cout << fragment_shader_filename << " is read to be empty !\n";
    fragment_shader_srcs.emplace_back(fragment_shader_src);
  }

  program* program_res = new program();
  GLuint my_program = glCreateProgram();

  for (const auto &fragment_shader_src : fragment_shader_srcs) {
    GLuint my_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    program_res->shaders_.emplace_back(my_fragment_shader);
    const GLchar *fragment_src = (const GLchar *) fragment_shader_src.c_str();
    glShaderSource(my_fragment_shader, 1, &fragment_src, 0);
    glCompileShader(my_fragment_shader);
    glAttachShader(my_program, my_fragment_shader);
  }

  GLuint my_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  program_res->shaders_.emplace_back(my_vertex_shader);
  const GLchar *vertex_src = (const GLchar *)vertex_shader_src.c_str();
  glShaderSource(my_vertex_shader, 1, &vertex_src, 0);
  glCompileShader(my_vertex_shader);

  if (!geometry_shader_filename.empty()) {
    const std::string geometry_shader_src = read_file(geometry_shader_filename, values_to_replace);
    if (geometry_shader_src.empty())
      std::cout << geometry_shader_filename << " is read to be empty !\n";
    GLuint my_geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
    program_res->shaders_.emplace_back(my_geometry_shader);
    const GLchar *geometry_src = (const GLchar*)geometry_shader_src.c_str();
    glShaderSource(my_geometry_shader, 1, &geometry_src, 0);
    glCompileShader(my_geometry_shader);
    glAttachShader(my_program, my_geometry_shader);
  }
  glAttachShader(my_program, my_vertex_shader);
  glLinkProgram(my_program);

  program_res->my_program_ = my_program;
  return program_res;

}

program *program::make_program(const std::string &vertex_shader_filename, const std::string& fragment_shader_filenames,
                             const std::string &geometry_shader_filename , const std::map<std::string, std::string>& values_to_replace) {
  return make_program_multiple_files(vertex_shader_filename, {fragment_shader_filenames}, geometry_shader_filename, values_to_replace);
}

std::string program::get_log() {
  std::string log_final;
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

void program::set_uniform_vector_float(const std::string &var_name, size_t count, float* pointer) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1fv(uniform_location, count, pointer);
}

void program::set_uniform_vector_float(const std::string &var_name, const std::vector<float>& vec) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1fv(uniform_location, vec.size(), vec.data());
}

void program::set_uniform_bool(const std::string &var_name, bool value) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1i(uniform_location, value);
}

void program::set_uniform_vector_vec3(const std::string& var_name, const std::vector<glm::vec3>& vec) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform3fv(uniform_location, vec.size(), glm::value_ptr(vec[0]));
}

void program::set_uniform_vector_vec3(const std::string& var_name, size_t size, void* data) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform3fv(uniform_location, size, static_cast<const GLfloat *>(data));
}

void program::set_uniform_vector_bool(const std::string& var_name, const std::vector<GLint>& vec) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1iv(uniform_location, vec.size(), vec.data());
}

void program::set_uniform_vector_bool(const std::string& var_name, size_t size, void* data) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1iv(uniform_location, size, static_cast<const GLint *>(data));
}

void program::set_uniform_vector_int(const std::string& var_name, size_t size, void* data) {
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  this->use();
  glUniform1iv(uniform_location, size, static_cast<const GLint *>(data));
}

void program::set_uniform_vector_mat4(const std::string& var_name, const std::vector<glm::mat4>& data) {
  this->use();
  int uniform_location = glGetUniformLocation(this->my_program_, var_name.c_str());
  glUniformMatrix4fv(uniform_location, data.size(), GL_FALSE, reinterpret_cast<const GLfloat *>(data.data()));
  //std::cout << uniform_location << ' ' << var_name << std::endl;
  //glCheckError();
  /*for (size_t i = 0; i < data.size(); ++i) {
    std::string var_name_indexed = var_name + "[" + std::to_string(i) + "]";
    int uniform_location = glGetUniformLocation(this->my_program_, var_name_indexed.c_str());

    glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(data[i]));
  }*/
  //glCheckError();
}