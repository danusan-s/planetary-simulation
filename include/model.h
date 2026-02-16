#ifndef MODEL_H
#define MODEL_H

#include <array>
#include <glad/glad.h>
#include <sstream>
#include <string>
#include <vector>

// Parser and initializer for WavefrontOBJ files to be used as 3D models.
class Model {
public:
  GLuint VAO, VBO;
  // Holds x y z nx ny nz u v for each vertex
  std::vector<std::array<float, 8>> vertices;

  Model();

  // generates model from obj data
  void Generate(const std::string &data);
};

#endif
