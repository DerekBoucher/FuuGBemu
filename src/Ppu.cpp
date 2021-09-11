#include "Ppu.hpp"

Ppu::Ppu() {

    auto x = 0;
    auto y = 0;

    // Position vertices required for a square / pixel.
    // Here every pair of float represents an x-y coordinate (i.e. single vertex).
    // The weird math is to convert from regular video coordinates to
    // openGL normalized device coordinates.
    for (auto i = 0; i < (NATIVE_SIZE_X * NATIVE_SIZE_Y * 12); i += 12) {
        positionVertices[i] = (-1.0f + (xDiv * x));
        positionVertices[i + 1] = (1.0f - (yDiv * y));
        positionVertices[i + 2] = (-1.0f + (xDiv * x) + xDiv);
        positionVertices[i + 3] = (1.0f - (yDiv * y));
        positionVertices[i + 4] = (-1.0f + (xDiv * x));
        positionVertices[i + 5] = (1.0f - (yDiv * y) - yDiv);
        positionVertices[i + 6] = (-1.0f + (xDiv * x) + xDiv);
        positionVertices[i + 7] = (1.0f - (yDiv * y));
        positionVertices[i + 8] = (-1.0f + (xDiv * x));
        positionVertices[i + 9] = (1.0f - (yDiv * y) - yDiv);
        positionVertices[i + 10] = (-1.0f + (xDiv * x) + xDiv);
        positionVertices[i + 11] = (1.0f - (yDiv * y) - yDiv);

        x++;

        // There are 12 vertices per pixel, therefore we need to verify
        // that we traversed a full scanline when x reaches 160.
        if (x == NATIVE_SIZE_X) {
            x = 0;
            y++;
        }
    }
}

Ppu::~Ppu() {}

void Ppu::SetContext(GLFWwindow* context) {
    window = context;
}

void Ppu::SetMemory(Memory* memory) {
    memoryRef = memory;
}

void Ppu::InitializeGLBuffers() {
    positionVBO = std::unique_ptr<Vbo>(new Vbo());
    colorVBO = std::unique_ptr<Vbo>(new Vbo());
    vao = std::unique_ptr<Vao>(new Vao());

    // Generate the position vertex buffer (remains static)
    positionVBO->Generate(positionVertices, sizeof(positionVertices));

    // Add the vertex buffers to the vertex array
    vao->AddBuffer(*positionVBO.get(), {0, 2, GL_FLOAT, sizeof(GLfloat) * 2, NULL});
    vao->AddBuffer(*colorVBO.get(), {1, 3, GL_FLOAT, sizeof(GLfloat) * 3, NULL});
}

void Ppu::BindContext() {
    glfwMakeContextCurrent(window);
}

void Ppu::UnBindContext() {
    glfwMakeContextCurrent(NULL);
}

void Ppu::Render() {
    DrawPixels();
    glFlush();
    SwapBuffers();
}

void Ppu::AttachShaders(Shader& vs, Shader& fs) {
    ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, vs.ID);
    glAttachShader(ShaderProgram, fs.ID);
    glLinkProgram(ShaderProgram);
    glValidateProgram(ShaderProgram);
    glUseProgram(ShaderProgram);
}

void Ppu::DrawPixels() {
    // Here we define a vec3 color vertex for every vec2 position
    // vertex that we defined above (i.e. we need 18 floats).
    GLfloat colorVertices[NATIVE_SIZE_X * NATIVE_SIZE_Y * 18];
    auto x = 0;
    auto y = 0;
    for (auto i = 0; i < (NATIVE_SIZE_X * NATIVE_SIZE_Y * 18); i += 18) {
        colorVertices[i] = pixels[x][y].r / 255.0f;
        colorVertices[i + 1] = pixels[x][y].g / 255.0f;
        colorVertices[i + 2] = pixels[x][y].b / 255.0f;
        colorVertices[i + 3] = pixels[x][y].r / 255.0f;
        colorVertices[i + 4] = pixels[x][y].g / 255.0f;
        colorVertices[i + 5] = pixels[x][y].b / 255.0f;
        colorVertices[i + 6] = pixels[x][y].r / 255.0f;
        colorVertices[i + 7] = pixels[x][y].g / 255.0f;
        colorVertices[i + 8] = pixels[x][y].b / 255.0f;
        colorVertices[i + 9] = pixels[x][y].r / 255.0f;
        colorVertices[i + 10] = pixels[x][y].g / 255.0f;
        colorVertices[i + 11] = pixels[x][y].b / 255.0f;
        colorVertices[i + 12] = pixels[x][y].r / 255.0f;
        colorVertices[i + 13] = pixels[x][y].g / 255.0f;
        colorVertices[i + 14] = pixels[x][y].b / 255.0f;
        colorVertices[i + 15] = pixels[x][y].r / 255.0f;
        colorVertices[i + 16] = pixels[x][y].g / 255.0f;
        colorVertices[i + 17] = pixels[x][y].b / 255.0f;

        x++;
        if (x == (NATIVE_SIZE_X)) {
            x = 0;
            y++;
        }
    }

    colorVBO->Generate(colorVertices, sizeof(colorVertices));

    // Finally, draw the pixel
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, NATIVE_SIZE_X * NATIVE_SIZE_Y * 12);
    vao->UnBind();
}

void Ppu::DrawPixel(GLuint x, GLuint y, uBYTE r, uBYTE g, uBYTE b) {    
    // Position vertices required for a square / pixel.
    // Here every pair of float represents an x-y coordinate (i.e. single vertex).
    // The weird math is to convert from regular video coordinates to
    // openGL normalized device coordinates.
    GLfloat positionVertices[12] = {
        (-1.0f + (xDiv * x)), (1.0f - (yDiv * y)),
        (-1.0f + (xDiv * x) + xDiv), (1.0f - (yDiv * y)),
        (-1.0f + (xDiv * x)), (1.0f - (yDiv * y) - yDiv),
        (-1.0f + (xDiv * x) + xDiv), (1.0f - (yDiv * y)),
        (-1.0f + (xDiv * x)), (1.0f - (yDiv * y) - yDiv),
        (-1.0f + (xDiv * x) + xDiv), (1.0f - (yDiv * y) - yDiv) 
    };

    // Here we define a vec3 color vertex for every vec2 position
    // vertex that we defined above (i.e. we need 18 floats).
    GLfloat colorVertices[18] = {
        r / 255.0f, g / 255.0f, b / 255.0f,
        r / 255.0f, g / 255.0f, b / 255.0f,
        r / 255.0f, g / 255.0f, b / 255.0f,
        r / 255.0f, g / 255.0f, b / 255.0f,
        r / 255.0f, g / 255.0f, b / 255.0f,
        r / 255.0f, g / 255.0f, b / 255.0f
    };

    positionVBO->Generate(positionVertices, sizeof(positionVertices));
    colorVBO->Generate(colorVertices, sizeof(colorVertices));

    // Finally, draw the pixel
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao->UnBind();
}

void Ppu::SwapBuffers() {
    glfwSwapBuffers(window);
}

void Ppu::DrawScanline()
{
    LCDC = GetLCDC();

    if (LCDC & (1 << 0))
    {
        RenderTiles();
    }
    if (LCDC & (1 << 1))
    {
        RenderSprites();
    }
}

void Ppu::UpdateGraphics(int cycles) {
    LCDC = GetLCDC();

    SetLCDStatus();

    if (LCDC & (1 << 7))
    {
        scanlineCounter -= cycles;
    }
    else
    {
        return;
    }

    if (scanlineCounter <= 0) // Time to render new frame
    {
        currentScanline = memoryRef->DmaRead(LY_ADR);

        scanlineCounter = 456;

        if (currentScanline < 144)
        {
            DrawScanline();
        }
        else if (currentScanline >= 144 && currentScanline < 154)
        {
            memoryRef->RequestInterupt(VBLANK_INT);
        }
        else
        {
            memoryRef->DmaWrite(LY_ADR, 0x00);
            return;
        }

        memoryRef->DmaWrite(LY_ADR, memoryRef->DmaRead(LY_ADR) + 1);
    }
}

void Ppu::RenderTiles()
{
    LCDC = GetLCDC();

    uWORD tileDataPtr = 0x0;
    uWORD tileMapPtr = 0x0;

    bool windowEnabled = false;
    bool unsignedID = false;

    // Determine The offsets to use when retrieving the Tile Identifiers from
    // Tile Map address space.
    uBYTE scrollX = memoryRef->DmaRead(SCR_X_ADR);
    uBYTE scrollY = memoryRef->DmaRead(SCR_Y_ADR);
    uBYTE winX = memoryRef->DmaRead(0xFF4B) - 7;
    uBYTE winY = memoryRef->DmaRead(0xFF4A);

    // Determine base address of tile data for BG & window
    if (LCDC & (1 << 4))
    {
        tileDataPtr = 0x8000;
        unsignedID = true;
    }
    else
    {
        tileDataPtr = 0x9000;
        unsignedID = false;
    }

    // Determine the base address tile Mappings for BG & window
    if (LCDC & (1 << 3))
    {
        tileMapPtr = 0x9C00;
    }
    else
    {
        tileMapPtr = 0x9800;
    }

    if (LCDC & (1 << 5))
    {
        windowEnabled = true;
    }
    else
    {
        windowEnabled = false;
    }

    // Determine the current scanline we are on
    currentScanline = memoryRef->DmaRead(LY_ADR);

    // Calculate which row in the tile to render
    uBYTE yPos;

    if (!windowEnabled)
        yPos = scrollY + currentScanline;
    else
        yPos = currentScanline - winY;

    uWORD tileRow = (yPos / 8) * 32;

    // Start Rendering the scanline
    for (int pixel = 0; pixel < 160; pixel++)
    {
        if (currentScanline < 0 || currentScanline > 144)
        {
            continue;
        }

        uBYTE xPos = pixel + (scrollX / 8);

        if (windowEnabled)
        {
            if (pixel >= winX)
            {
                xPos = pixel - winX;
            }
        }

        uWORD tileColumn = xPos / 8;

        // Determine the address for the tile ID
        uWORD currentTileIDAdr = tileMapPtr + tileColumn + tileRow;

        // Fetch the tile ID
        uBYTE tileID = memoryRef->DmaRead(currentTileIDAdr);

        // Determine the current pixel data from the tile data
        uWORD tileLineOffset = (yPos % 8) * 2; //Each line is 2 bytes
        uWORD tileDataAdr;

        if (unsignedID)
        {
            tileDataAdr = tileDataPtr + (tileID * 16);
        }
        else
        {
            if (tileID & 0x80)
            {
                tileID = ~tileID;
                tileID += 1;
                tileDataAdr = tileDataPtr - (tileID * 16);
            }
            else
            {
                tileDataAdr = tileDataPtr + (tileID * 16);
            }
        }

        uBYTE data1 = memoryRef->DmaRead(tileDataAdr + tileLineOffset);
        uBYTE data2 = memoryRef->DmaRead(tileDataAdr + tileLineOffset + 1);

        int currentBitPosition = (((pixel % 8) - 7) * -1);

        uBYTE ColorCode = 0x00;

        if (data2 & (1 << currentBitPosition))
        {
            ColorCode |= 0x02;
        }

        if (data1 & (1 << currentBitPosition))
        {
            ColorCode |= 0x01;
        }

        uBYTE R = 0x00;
        uBYTE G = 0x00;
        uBYTE B = 0x00;

        uBYTE Color_00 = memoryRef->DmaRead(0xFF47) & 0x03;
        uBYTE Color_01 = ((memoryRef->DmaRead(0xFF47) >> 2) & 0x03);
        uBYTE Color_10 = ((memoryRef->DmaRead(0xFF47) >> 4) & 0x03);
        uBYTE Color_11 = ((memoryRef->DmaRead(0xFF47) >> 6) & 0x03);

        // Determine actual color for pixel via Color Pallete reg
        switch (ColorCode)
        {
        case 0x00:
            if (Color_00 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_00 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_00 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_00 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        case 0x01:
            if (Color_01 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_01 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_01 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_01 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        case 0x02:
            if (Color_10 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_10 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_10 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_10 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        case 0x03:
            if (Color_11 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_11 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_11 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_11 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        default:
            break;
        }

        if (currentScanline < 0 || currentScanline > 143 || pixel < 0 || pixel > 159)
        {
            continue;
        }

        pixels[pixel][currentScanline] = {R, G, B};

        // Update pixel data
        pixelData[pixel][currentScanline] = ColorCode;
    }
}

void Ppu::RenderWindow()
{
    LCDC = GetLCDC();

    uBYTE winY = memoryRef->DmaRead(0xFF4A);
    uBYTE winX = memoryRef->DmaRead(0xFF4B) - 7;
    uWORD tileMapPtr = 0x9800;
    uWORD tileDataPtr = 0x9000;
    bool unsignedID = false;

    if (LCDC & (1 << 6))
    {
        tileMapPtr = 0x9C00;
    }

    if (LCDC & (1 << 4))
    {
        tileDataPtr = 0x8000;
        unsignedID = true;
    }

    // Determine the current scanline we are on
    currentScanline = memoryRef->DmaRead(LY_ADR);

    // Calculate which row in the tile to render
    uBYTE yPos = winY + currentScanline;
    uWORD tileRow = (yPos / 8) * 32;

    // Start Rendering the scanline
    for (int pixel = 0; pixel < 160; pixel++)
    {
        uBYTE xPos = pixel + winX;

        uWORD tileColumn = xPos / 8;

        // Determine the address for the tile ID
        uWORD currentTileMapAdr = tileMapPtr + tileColumn + tileRow;

        // Fetch the tile ID
        uBYTE tileID = memoryRef->DmaRead(currentTileMapAdr);

        // Determine the current pixel data from the tile data
        uWORD tileLineOffset = (yPos % 8) * 2; //Each line is 2 bytes
        uWORD tileDataAdr;

        if (unsignedID)
        {
            tileDataAdr = tileDataPtr + (tileID * 16);
        }
        else
        {
            if (tileID & 0x80)
            {
                tileID = ~tileID;
                tileID += 1;
                tileDataAdr = tileDataPtr - (tileID * 16);
            }
            else
            {
                tileDataAdr = tileDataPtr + (tileID * 16);
            }
        }

        uBYTE data1 = memoryRef->DmaRead(tileDataAdr + tileLineOffset);
        uBYTE data2 = memoryRef->DmaRead(tileDataAdr + tileLineOffset + 1);

        int currentBitPosition = (((pixel % 8) - 7) * -1);

        uBYTE ColorCode = 0x00;

        if (data2 & (1 << currentBitPosition))
        {
            ColorCode |= 0x02;
        }

        if (data1 & (1 << currentBitPosition))
        {
            ColorCode |= 0x01;
        }

        uBYTE R = 0x00;
        uBYTE G = 0x00;
        uBYTE B = 0x00;

        uBYTE Color_00 = memoryRef->DmaRead(0xFF47) & 0x03;
        uBYTE Color_01 = ((memoryRef->DmaRead(0xFF47) >> 2) & 0x03);
        uBYTE Color_10 = ((memoryRef->DmaRead(0xFF47) >> 4) & 0x03);
        uBYTE Color_11 = ((memoryRef->DmaRead(0xFF47) >> 6) & 0x03);

        // Determine actual color for pixel via Color Pallete reg
        switch (ColorCode)
        {
        case 0x00:
            if (Color_00 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_00 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_00 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_00 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        case 0x01:
            if (Color_01 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_01 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_01 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_01 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        case 0x02:
            if (Color_10 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_10 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_10 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_10 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        case 0x03:
            if (Color_11 == 0x00)
            {
                R = 245;
                G = 245;
                B = 245;
            }
            else if (Color_11 == 0x1)
            {
                R = 211;
                G = 211;
                B = 211;
            }
            else if (Color_11 == 0x2)
            {
                R = 169;
                G = 169;
                B = 169;
            }
            else if (Color_11 == 0x3)
            {
                R = 0;
                G = 0;
                B = 0;
            }
            break;
        default:
            break;
        }

        if (currentScanline < 0 || currentScanline > 143 || pixel < 0 || pixel > 159)
        {
            continue;
        }

        pixels[pixel][currentScanline] = {R, G, B};

        // Update pixel data
        pixelData[pixel][currentScanline] = ColorCode;
    }
}

void Ppu::RenderSprites()
{
    LCDC = GetLCDC();

    bool u_8x16 = false;

    if (LCDC & (1 << 2))
    {
        u_8x16 = true;
    }

    sprite *sprites = ProcessSprites();

    for (uBYTE i = 0; i < 40; i++)
    {
        bool yFlip = (sprites[i].attributes & (1 << 6));
        bool xFlip = (sprites[i].attributes & (1 << 5));
        bool priority = !(sprites[i].attributes & (1 << 7));

        currentScanline = memoryRef->DmaRead(LY_ADR);

        uBYTE ysize = 8;

        if (u_8x16)
        {
            sprites[i].patternNumber &= 0xFE;
            ysize = 16;
        }

        if ((currentScanline >= sprites[i].yPos) && (currentScanline < (sprites[i].yPos + ysize)))
        {
            int line = currentScanline - sprites[i].yPos;

            if (yFlip)
            {
                line -= ysize;
                line *= -1;
            }

            line *= 2;

            uWORD dataaddr = (0x8000 + (sprites[i].patternNumber * 16)) + line;
            uBYTE data1 = memoryRef->DmaRead(dataaddr);
            uBYTE data2 = memoryRef->DmaRead(dataaddr + 1);

            for (int tilepixel = 7; tilepixel >= 0; tilepixel--)
            {
                int ColorBit = tilepixel;

                if (xFlip)
                {
                    ColorBit -= 7;
                    ColorBit *= -1;
                }

                uBYTE ColorCode = 0x00;

                if (data2 & (1 << ColorBit))
                {
                    ColorCode |= 0x02;
                }

                if (data1 & (1 << ColorBit))
                {
                    ColorCode |= 0x01;
                }

                uWORD coloradr = 0x0000;

                if (sprites[i].attributes & (1 << 4))
                {
                    coloradr = 0xFF49;
                }
                else
                {
                    coloradr = 0xFF48;
                }

                uBYTE R = 0x0;
                uBYTE G = 0x0;
                uBYTE B = 0x0;

                uBYTE Color_00 = (memoryRef->DmaRead(coloradr) & 0x03);
                uBYTE Color_01 = ((memoryRef->DmaRead(coloradr) >> 2) & 0x03);
                uBYTE Color_10 = ((memoryRef->DmaRead(coloradr) >> 4) & 0x03);
                uBYTE Color_11 = ((memoryRef->DmaRead(coloradr) >> 6) & 0x03);

                //Determine actual color for pixel via Color Pallete reg
                switch (ColorCode)
                {
                case 0x00:
                    if (Color_00 == 0x00)
                    {
                        R = 245;
                        G = 245;
                        B = 245;
                    }
                    else if (Color_00 == 0x1)
                    {
                        R = 211;
                        G = 211;
                        B = 211;
                    }
                    else if (Color_00 == 0x2)
                    {
                        R = 169;
                        G = 169;
                        B = 169;
                    }
                    else if (Color_00 == 0x3)
                    {
                        R = 0;
                        G = 0;
                        B = 0;
                    }
                    break;
                case 0x01:
                    if (Color_01 == 0x00)
                    {
                        R = 245;
                        G = 245;
                        B = 245;
                    }
                    else if (Color_01 == 0x1)
                    {
                        R = 211;
                        G = 211;
                        B = 211;
                    }
                    else if (Color_01 == 0x2)
                    {
                        R = 169;
                        G = 169;
                        B = 169;
                    }
                    else if (Color_01 == 0x3)
                    {
                        R = 0;
                        G = 0;
                        B = 0;
                    }
                    break;
                case 0x02:
                    if (Color_10 == 0x00)
                    {
                        R = 245;
                        G = 245;
                        B = 245;
                    }
                    else if (Color_10 == 0x1)
                    {
                        R = 211;
                        G = 211;
                        B = 211;
                    }
                    else if (Color_10 == 0x2)
                    {
                        R = 169;
                        G = 169;
                        B = 169;
                    }
                    else if (Color_10 == 0x3)
                    {
                        R = 0;
                        G = 0;
                        B = 0;
                    }
                    break;
                case 0x03:
                    if (Color_11 == 0x00)
                    {
                        R = 245;
                        G = 245;
                        B = 245;
                    }
                    else if (Color_11 == 0x1)
                    {
                        R = 211;
                        G = 211;
                        B = 211;
                    }
                    else if (Color_11 == 0x2)
                    {
                        R = 169;
                        G = 169;
                        B = 169;
                    }
                    else if (Color_11 == 0x3)
                    {
                        R = 0;
                        G = 0;
                        B = 0;
                    }
                    break;
                default:
                    break;
                }

                if (R == 224 && G == 248 && B == 208)
                {
                    continue;
                }

                int xPix = 0 - tilepixel - 1;

                int pixel = sprites[i].xPos + xPix;

                if (currentScanline < 0 || currentScanline > 143 || pixel < 0 || pixel > 159 || ColorCode == 0x00)
                {
                    continue;
                }

                // Determine if sprite pixel has priority over background or window
                if (!priority && pixelData[pixel][currentScanline] != 0x00)
                {
                    continue;
                }

                pixels[pixel][currentScanline] = {R, G, B};
            }
        }
    }
}

void Ppu::SetLCDStatus()
{
    LCDC = GetLCDC();
    STAT = GetStat();

    if (!(LCDC & (1 << 7)))
    {
        scanlineCounter = 456;
        memoryRef->DmaWrite(LY_ADR, 0x00);
        STAT &= 0xFC;
        memoryRef->DmaWrite(STAT_ADR, STAT);
        return;
    }

    currentScanline = memoryRef->DmaRead(LY_ADR);
    uBYTE currentMode = STAT & 0x03;

    uBYTE mode = 0;
    bool reqInt = false;

    // Mode 1
    if (currentScanline >= 144)
    {
        mode = 0x01;
        STAT &= 0xFC;
        STAT |= mode;
        reqInt = (STAT & (1 << 5));
    }
    else
    {
        int mode2BOUND = 456 - 80;
        int mode3BOUND = mode2BOUND - 172;

        // Mode 2
        if (scanlineCounter >= mode2BOUND)
        {
            mode = 0x02;
            STAT &= 0xFC;
            STAT |= mode;
            reqInt = (STAT & (1 << 5));
        }
        // Mode 3
        else if (scanlineCounter >= mode3BOUND)
        {
            mode = 0x03;
            STAT &= 0xFC;
            STAT |= mode;
        }
        // Mode 0
        else
        {
            mode = 0x00;
            STAT &= 0xFC;
            STAT |= mode;
            reqInt = (STAT & (1 << 3));
        }
    }

    if (reqInt && (mode != currentMode))
    {
        memoryRef->RequestInterupt(LCDC_INT);
    }

    if (memoryRef->DmaRead(LY_ADR) == memoryRef->DmaRead(LYC_ADR))
    {
        STAT |= (1 << 2);
        if (STAT & (1 << 6))
        {
            memoryRef->RequestInterupt(LCDC_INT);
        }
    }
    else
    {
        STAT &= 0xFB;
    }

    memoryRef->DmaWrite(STAT_ADR, STAT);
}

uBYTE Ppu::GetLCDC()
{
    return memoryRef->DmaRead(LCDC_ADR);
}

uBYTE Ppu::GetStat()
{
    return memoryRef->DmaRead(STAT_ADR);
}

Ppu::sprite *Ppu::ProcessSprites()
{
    sprite *processedSprites = new sprite[40];

    for (uBYTE i = 40; i != 255; i--)
    {
        uBYTE index = i * 4;

        processedSprites[i].yPos = memoryRef->DmaRead(OAM_ADR + index) - 0x10;
        processedSprites[i].xPos = memoryRef->DmaRead(OAM_ADR + index + 1);
        processedSprites[i].patternNumber = memoryRef->DmaRead(OAM_ADR + index + 2);
        processedSprites[i].attributes = memoryRef->DmaRead(OAM_ADR + index + 3);
    }

    // for(uBYTE i = 0; i < 40; i++) {
    //     for(uBYTE j = 0; j < 40; j++) {
    //         if (i == j) {
    //             continue;
    //         }

    //         for (uBYTE k = 1; k < 8; k++) {
    //             if ((processedSprites[j].xPos + k) == processedSprites[i].xPos) {
    //                 temp = processedSprites[i];
    //                 processedSprites[i] = processedSprites[j];
    //                 processedSprites[j] = temp;
    //                 break;
    //             }
    //         }
    //     }
    // }

    return processedSprites;
}

