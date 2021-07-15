#pragma once
#include "program.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// This class was created following the tutorial of www.learnopengl.com
// It was modified to satisfy our needs afterwards
class Model {
 public:
  Model();
  Model(const std::string& path);
  void draw(program* program);

  std::vector<Mesh> meshes;
  std::string directory;
  std::vector<Texture> textures_loaded;//To avoid loading multiples times the same image
 private:
  void loadModel(const std::string& path);
  void processNode(aiNode* node, const aiScene* scene);
  Mesh processMesh(aiMesh* mesh, const aiScene* scene);
  std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, const TextureType& typeName);
};