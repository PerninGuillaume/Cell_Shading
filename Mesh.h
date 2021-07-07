#pragma once
#include "program.h"

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

enum TextureType {
  DIFFUSE,
  SPECULAR
};

struct Texture {
  unsigned int id;
  TextureType texture_type;
  std::string path;
};

class Mesh {
 public:
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, glm::vec3 color);
  void draw(program* program);
  void setupMesh();

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f);
  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
};