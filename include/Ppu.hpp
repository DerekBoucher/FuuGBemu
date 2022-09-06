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
    void Render();
    void AttachShaders(Shader& vs, Shader& fs);
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
        uBYTE r, g, b, colorCode;

        pixel() {
            r = 0xFF;
            g = 0xFF;
            b = 0xFF;
            colorCode = 0xFF;
        };

        pixel(uBYTE R, uBYTE G, uBYTE B, uBYTE colorCode) {
            r = R;
            g = G;
            b = B;
            colorCode = colorCode;
        };
    };

    pixel** pixels;
    uBYTE LCDC;
    uBYTE STAT;
    Memory* memoryRef;
    int currentScanline;
    int scanlineCounter;

    void DrawScanline();
    void RenderTiles();
    void RenderSprites();
    void SetLCDStatus();
    void DrawPixels();
    pixel DeterminePixelRGB(uBYTE colorCode, uWORD colorAdr);
    sprite* ProcessSprites();
    uBYTE GetStat();
    uBYTE GetLCDC();

    GLfloat* positionVertices;

    Vao* vao;
    Vbo* positionVBO;
    Vbo* colorVBO;

    constexpr static GLfloat xDiv = 2.0 / 160;
    constexpr static GLfloat yDiv = 2.0 / 144;

    GLuint ShaderProgram;
};

#endif
