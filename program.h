#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>

class program {
public:
    program();
    ~program();
    static program *make_program(const std::string &vertex_shader_filename, const std::string &fragment_shader_filename);
    std::string get_log();
    bool is_ready();
    void use();
    void set_uniform_float(const std::string& var_name, float value);
    void set_uniform_vec3(const std::string& var_name, float x, float y, float z);
    void set_uniform_vec3(const std::string& var_name, const glm::vec3& vec3);
    void set_uniform_vec3(const std::string &var_name, float val);
    void set_uniform_int(const std::string &var_name, int value);
    void set_uniform_mat4(const std::string &var_name, const glm::mat4& mat);

    GLuint my_program_ = 0;
    std::vector<GLint> shaders_;

};