#ifndef PPU_H
#define PPU_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

#include "opengl/Vao.hpp"
#include "opengl/Vbo.hpp"
#include "opengl/Shader.hpp"
#include "Memory.hpp"

#define NATIVE_SIZE_X 160
#define NATIVE_SIZE_Y 144
#define SCALE 5

typedef unsigned char uBYTE;

class Ppu {

public:
    Ppu();
    ~Ppu();

    void UpdateGraphics(int);
    void SetContext(GLFWwindow* parent);
    void BindContext();
    void UnBindContext();
    void Render();
    void AttachShaders(Shader& vs, Shader& fs);
    void SwapBuffers();
    void SetMemory(Memory* memory);
    void InitializeGLBuffers();

private:

    struct sprite {
        uBYTE yPos;
        uBYTE xPos;
        uBYTE patternNumber;
        uBYTE attributes;
    };

    struct pixel {
        uBYTE r, g, b;
    };
    
    pixel pixels[NATIVE_SIZE_X][NATIVE_SIZE_Y];
    uBYTE pixelData[NATIVE_SIZE_X][NATIVE_SIZE_Y];
    uBYTE LCDC;
    uBYTE STAT;
    Memory *memoryRef;
    int currentScanline;
    int scanlineCounter;

    void DrawScanline();
    void RenderTiles();
    void RenderWindow();
    void RenderSprites();
    void SetLCDStatus();
    void DrawPixels();
    void DrawPixel(GLuint x, GLuint y, uBYTE r, uBYTE g, uBYTE b);
    sprite *ProcessSprites();
    uBYTE GetStat();
    uBYTE GetLCDC();

    GLfloat positionVertices[NATIVE_SIZE_X * NATIVE_SIZE_Y * 12];

    std::unique_ptr<Vao> vao;
    std::unique_ptr<Vbo> positionVBO, colorVBO;

    constexpr static GLfloat xDiv = 2.0 / 160;
    constexpr static GLfloat yDiv = 2.0 / 144;

    GLFWwindow* window;
    GLuint ShaderProgram;
};

#endif
