#ifndef CUBEMAP_H
#define CUBEMAP_H
#include <glad/glad.h>
#include <vector>

class Cubemap {
public:
  // holds the ID of the texture object, used for all texture operations to
  // reference to this particular texture
  unsigned int ID;
  // texture image dimensions
  std::vector<unsigned int> Width,
      Height; // width and height of loaded image in pixels
  // texture Format
  unsigned int Internal_Format; // format of texture object
  unsigned int Image_Format;    // format of loaded image
  // texture configuration
  unsigned int Wrap_S;     // wrapping mode on S axis
  unsigned int Wrap_T;     // wrapping mode on T axis
  unsigned int Wrap_R;     // wrapping mode on R axis
  unsigned int Filter_Min; // filtering mode if texture pixels < screen pixels
  unsigned int Filter_Max; // filtering mode if texture pixels > screen pixels
  // constructor (sets default texture modes)
  Cubemap();
  // generates texture from image data
  void Generate(std::vector<unsigned int> width,
                std::vector<unsigned int> height,
                std::vector<unsigned char *> data);
  // binds the texture as the current active GL_TEXTURE_2D texture object
  void Bind() const;
};

#endif // !CUBEMAP_H
