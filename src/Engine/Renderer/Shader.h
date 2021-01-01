#ifndef MINECRAFTCLONE_SHADER_H
#define MINECRAFTCLONE_SHADER_H


#include <string>

struct Shader {
unsigned int rendererID;

  Shader() = default;
  Shader(unsigned int type, const char* shaderSource);
  Shader(unsigned int type, std::string fileName);
  unsigned int getID();
  ~Shader();
};


#endif //MINECRAFTCLONE_SHADER_H