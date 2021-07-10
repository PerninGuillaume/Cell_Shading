#pragma once
#include "program.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
 public:
  Model(const std::string& path);
  void draw(program* program);

  std::vector<Mesh> meshes;
  std::string directory;
  std::vector<Texture> textures_loaded;//To avoid loading multiples times the same image
 private:
  bool not_printed_before = true;
  void loadModel(const std::string& path);
  void processNode(aiNode* node, const aiScene* scene);
  Mesh processMesh(aiMesh* mesh, const aiScene* scene);
  std::vector<Texture> loadMaterialTextures(aiMaterial *material, aiTextureType type, const TextureType& typeName);


};