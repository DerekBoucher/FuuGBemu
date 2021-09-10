#ifndef VBO_H
#define VBO_H

#include <GL/glew.h>

class Vbo {

public:
    GLuint ID;
    Vbo();
    ~Vbo();

    void Bind();
    void Generate(GLfloat*, GLsizeiptr);
    void UnBind();
    void Delete();
};

#endif
