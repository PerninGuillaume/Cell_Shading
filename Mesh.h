#pragma once
#include "program.h"

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

enum TextureType {
  DIFFUSE,
};

struct Texture {
  unsigned int id;
  TextureType texture_type;
  std::string path;
};

// This class was created following the tutorial of www.learnopengl.com
// It was modified to satisfy our needs afterwards
class Mesh {
 public:
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 color,
       bool use_color);
  void draw(program* program);
  void setupMesh();

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
  bool use_color;
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
};