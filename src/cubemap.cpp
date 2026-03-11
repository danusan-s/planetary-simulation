#include "cubemap.h"
#include <cassert>
#include <iostream>

Cubemap::Cubemap()
    : Width(0), Height(0), Internal_Format(GL_RGB), Image_Format(GL_RGB),
      Wrap_S(GL_CLAMP_TO_EDGE), Wrap_T(GL_CLAMP_TO_EDGE),
      Wrap_R(GL_CLAMP_TO_EDGE), Filter_Min(GL_LINEAR), Filter_Max(GL_LINEAR) {
  glGenTextures(1, &this->ID);
}

void Cubemap::Generate(std::vector<unsigned int> width,
                       std::vector<unsigned int> height,
                       std::vector<unsigned char *> data) {
  this->Width = width;
  this->Height = height;
  // create Texture
  // Bind the texture
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->ID);

  // set Texture wrap and filter modes
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, this->Wrap_S);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, this->Wrap_T);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, this->Wrap_R);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, this->Filter_Max);

  assert(data.size() == 6); // Ensure we have 6 images for the cubemap

  for (unsigned int i = 0; i < 6; i++) {
    glTexImage2D(static_cast<GLenum>(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0,
                 this->Internal_Format, width[i], height[i], 0,
                 this->Image_Format, GL_UNSIGNED_BYTE, data[i]);

    while (GLenum err = glGetError())
      std::cout << "GL ERROR after uploading face " << i << ": " << err
                << std::endl;
  }

  // unbind texture
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Cubemap::Bind() const {
  glBindTexture(GL_TEXTURE_CUBE_MAP, this->ID);
}
