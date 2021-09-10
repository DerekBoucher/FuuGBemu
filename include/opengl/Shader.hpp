#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>

class Shader {

public:
    Shader(std::string filepath);
    ~Shader();

    GLuint ID;
};

#endif
