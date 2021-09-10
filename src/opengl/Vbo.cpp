#include "opengl/Vbo.hpp"

Vbo::Vbo() {
    glGenBuffers(1, &ID);
}

Vbo::~Vbo() {
    glDeleteBuffers(1, &ID);
}

void Vbo::Generate(GLfloat* vertices, GLsizeiptr size) {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Vbo::Bind() {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void Vbo::UnBind() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Vbo::Delete() {
    glDeleteBuffers(1, &ID);
}
