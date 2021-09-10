#ifndef VAO_H
#define VAO_H

#include "Vbo.hpp"
#include <GL/glew.h>

class Vao {

public:

    struct Layout {
        GLuint Index;
        GLint VertexComponents;
        GLenum Type;
        GLsizei Stride;
        const void* ElementOffset;
    };

    GLuint ID;
    Vao();
    ~Vao();

    void AddBuffer(Vbo& buffer, Layout layout);
    void Bind();
    void UnBind();
    void Delete();
};

#endif
