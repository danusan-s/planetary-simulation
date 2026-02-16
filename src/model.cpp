#include "model.h"

Model::Model() {
}

void Model::Generate(const std::string &data) {
  std::istringstream stream(data);
  std::string line;

  std::vector<std::array<float, 3>> positions;
  std::vector<std::array<float, 3>> normals;
  std::vector<std::array<float, 2>> texCoords;

  while (std::getline(stream, line)) {
    std::istringstream lineStream(line);
    std::string prefix;
    lineStream >> prefix;

    if (prefix == "v") {
      float x, y, z;
      lineStream >> x >> y >> z;
      positions.push_back({x, y, z});
    } else if (prefix == "vn") {
      float nx, ny, nz;
      lineStream >> nx >> ny >> nz;
      normals.push_back({nx, ny, nz});
    } else if (prefix == "vt") {
      float u, v;
      lineStream >> u >> v;
      texCoords.push_back({u, v});
    } else if (prefix == "f") {
      std::string vertexStr;
      while (lineStream >> vertexStr) {
        std::istringstream vertexStream(vertexStr);
        std::string posIndexStr, texIndexStr, normIndexStr;

        std::getline(vertexStream, posIndexStr, '/');
        std::getline(vertexStream, texIndexStr, '/');
        std::getline(vertexStream, normIndexStr, '/');

        int posIndex = std::stoi(posIndexStr) - 1;
        int texIndex = std::stoi(texIndexStr) - 1;
        int normIndex = std::stoi(normIndexStr) - 1;

        const auto &pos = positions[posIndex];
        const auto &norm = normals[normIndex];
        const auto &tex = texCoords[texIndex];

        vertices.push_back({pos[0], pos[1], pos[2], norm[0], norm[1], norm[2],
                            tex[0], tex[1]});
      }
    }
  }

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  // Position (location = 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Normal (location = 1)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // UV (location = 2)
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}
