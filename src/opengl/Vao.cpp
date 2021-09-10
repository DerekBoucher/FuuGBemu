#include "opengl/Vao.hpp"

Vao::Vao() {
    glGenVertexArrays(1, &ID);
}

Vao::~Vao() {
    glDeleteVertexArrays(1, &ID);
}

void Vao::AddBuffer(Vbo &vbo, Vao::Layout layout) {
    vbo.Bind();
    Bind();
    glEnableVertexAttribArray(layout.Index);
    glVertexAttribPointer(layout.Index, layout.VertexComponents, layout.Type, GL_FALSE, layout.Stride, layout.ElementOffset);
    UnBind();
    vbo.UnBind();
}

void Vao::Bind() {
    glBindVertexArray(ID);
}

void Vao::UnBind() {
    glBindVertexArray(0);
}

void Vao::Delete() {
    glDeleteVertexArrays(1, &ID);
}
