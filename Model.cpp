#include <iostream>
#include "Model.h"
#include "stb_image.h"

unsigned int loadTexture(const std::string& path, const std::string& directory);

Model::Model(const std::string &path) {
  loadModel(path);
}

void Model::draw(program* program) {
  for (auto mesh : meshes) {
    mesh.draw(program);
  }
}

void Model::loadModel(const std::string &path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    //TODO maybe throw an error here
    return;
  }
  directory = path.substr(0, path.find_last_of('/'));
  processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  for (unsigned int i = 0; i < node->mNumChildren; ++i) {
    aiNode* child = node->mChildren[i];
    processNode(child, scene);
  }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  bool use_color = true;
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  bool has_texture_Coords = mesh->mTextureCoords[0];
  bool has_vertex_color = mesh->HasVertexColors(0);
  if (has_vertex_color)
    std::cout << " Color : " << mesh->mColors[0]->r << mesh->mColors[0]->g << mesh->mColors[0]->b << std::endl;

  mesh->mColors;
  // Vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
    Vertex vertex;
    auto v = mesh->mVertices[i];
    vertex.Position = glm::vec3(v.x, v.y, v.z);

    auto n = mesh->mNormals[i];
    vertex.Normal = glm::vec3(n.x, n.y, n.z);

    if (has_texture_Coords) {
      auto t = mesh->mTextureCoords[0][i];
      vertex.TexCoords = glm::vec2(t.x, t.y);
    } else {
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
    }
    vertices.push_back(vertex);
  }

  //Indices
  for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      indices.push_back(face.mIndices[j]);
    }
  }

  aiColor3D color (0.f,0.f,0.f);
  //Textures coordinates
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    if (not_printed_before) {
      not_printed_before = false;
      for (unsigned int index = 0; index < material->mNumProperties; ++index) {
        auto material_property = material->mProperties[index];
        std::cout << material_property->mKey.C_Str() << std::endl;
        auto data = material_property->mData;
        std::cout << data;
      }
    }
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    material->Get(AI_MATKEY_COLOR_DIFFUSE,color);
    float opacity;
    //material->Get("mat.gltf.alphaCutOff", opacity);
    //std::cout << opacity << std::endl;
    //material->Get(AI_MATKEY_COLOR_SPECULAR,color);
    //material->Get(AI_MATKEY_COLOR_AMBIENT,color);

    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    /*
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::NORMAL);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
     */
  }
  glm::vec3 color_glm = glm::vec3(color.r, color.g, color.b);
  if (color_glm.r != 0 || color_glm.g != 0 || color_glm.b != 0) {
    std::cout << "Color found in material : " << color_glm.r << ' ' << color_glm.g << ' ' << color_glm.b << std::endl;
  }
  if (!textures.empty()) {
    use_color = false;
    std::cout << "Found at least one texture" << std::endl;
  }
  return Mesh(vertices, indices, textures, color_glm, use_color);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial *material, aiTextureType type, const TextureType& typeName) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < material->GetTextureCount(type); ++i) {
    aiString str;
    material->GetTexture(type, i, &str);
    bool already_loaded = false;
    for (auto& j : textures_loaded) {
      if (j.path == str.C_Str()) {
        textures.push_back(j);
        already_loaded = true;
        break;
      }
    }
    if (!already_loaded) {
      Texture texture;
      texture.id = loadTexture(str.C_Str(), directory);
      texture.texture_type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      textures_loaded.push_back(texture);
    }
  }
  return textures;
}

unsigned int loadTexture(const std::string& path, const std::string& directory)
{
  std::string filename = directory + '/' + path;
  std::cout << "Trying to load texture : " << filename << std::endl;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format = GL_RGB;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}
