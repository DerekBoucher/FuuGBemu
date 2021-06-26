#include "PPU.h"

PPU::PPU(wxWindow *screen, Memory *mem)
{
    WXWidget widget = screen->GetHandle();
#ifdef FUUGB_SYSTEM_LINUX
    GdkWindow *gdkWindow = gtk_widget_get_window(widget);
    Window x11Handle = GDK_WINDOW_XID(gdkWindow);
    sdlWindow = SDL_CreateWindowFrom((void *)x11Handle);
#endif

    if (sdlWindow == NULL)
    {
        wxPrintf("Error occured during SDL window creation: %s\n", SDL_GetError());
        SDL_Quit();
        wxExit();
    }

    renderer = SDL_GetRenderer(sdlWindow);

    if (renderer == NULL)
    {
        if (!(renderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_SOFTWARE)))
        {
            wxPrintf("Error occured during SDL renderer creation: %s\n", SDL_GetError());
            SDL_Quit();
            wxExit();
        }
    }

    memoryRef = mem;

    LCDC = GetLCDC();
    STAT = GetStat();

    currentScanline = 1;
    scanlineCounter = 456;
}

PPU::~PPU()
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_DestroyRenderer(renderer);
}

void PPU::RenderScreen()
{
    LCDC = GetLCDC();

    if (!(LCDC & (1 << 7)))
        return;

    SDL_RenderPresent(renderer);
}

void PPU::UpdateGraphics(int cycles)
{
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

void PPU::DrawScanline()
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

void PPU::RenderTiles()
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

        int w, h, dw, dh;

        SDL_GetWindowSize(sdlWindow, &w, &h);

        dw = w / NATIVE_SIZE_X;
        dh = h / NATIVE_SIZE_Y;

        SDL_Rect target;

        target.x = pixel * dw;
        target.y = currentScanline * dh;
        target.w = dw;
        target.h = dh;

        SDL_SetRenderDrawColor(renderer, R, G, B, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &target);
        SDL_RenderDrawRect(renderer, &target);

        // Update pixel data
        pixelData[pixel][currentScanline] = ColorCode;
    }
}

void PPU::RenderWindow()
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

        int w, h, dw, dh;

        SDL_GetWindowSize(sdlWindow, &w, &h);

        dw = w / NATIVE_SIZE_X;
        dh = h / NATIVE_SIZE_Y;

        SDL_Rect target;

        target.x = pixel * dw;
        target.y = currentScanline * dh;
        target.w = dw;
        target.h = dh;

        SDL_SetRenderDrawColor(renderer, R, G, B, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &target);
        SDL_RenderDrawRect(renderer, &target);

        // Update pixel data
        pixelData[pixel][currentScanline] = ColorCode;
    }
}

void PPU::RenderSprites()
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

                int w, h, dw, dh;

                SDL_GetWindowSize(sdlWindow, &w, &h);

                dw = w / NATIVE_SIZE_X;
                dh = h / NATIVE_SIZE_Y;

                SDL_Rect target;

                target.x = pixel * dw;
                target.y = currentScanline * dh;
                target.w = dw;
                target.h = dh;

                SDL_SetRenderDrawColor(renderer, R, G, B, SDL_ALPHA_OPAQUE);
                SDL_RenderFillRect(renderer, &target);
                SDL_RenderDrawRect(renderer, &target);
            }
        }
    }
}

void PPU::SetLCDStatus()
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

uBYTE PPU::GetLCDC()
{
    return memoryRef->DmaRead(LCDC_ADR);
}

uBYTE PPU::GetStat()
{
    return memoryRef->DmaRead(STAT_ADR);
}

PPU::sprite *PPU::ProcessSprites()
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

void PPU::RenderPixel(int x, int y, uBYTE R, uBYTE G, uBYTE B)
{
    SDL_Surface *surface = SDL_GetWindowSurface(sdlWindow);
    SDL_LockSurface(surface);

    int h = surface->h;
    int w = surface->w;

    int dh = h / NATIVE_SIZE_Y;
    int dw = w / NATIVE_SIZE_X;

    SDL_Rect target;
    target.x = x * dw;
    target.y = y * dh;
    target.w = dw;
    target.h = dh;

    std::vector<uint8_t> pixels(h * surface->pitch, 0);

    for (int i = 0; i < dw; i++)
    {
        for (int j = 0; j < dh; j++)
        {
            pixels[(x * dw) + (y * dh)] = R;
            pixels[(x * dw) + (y * dh)] = G;
            pixels[(x * dw) + (y * dh)] = B;
        }
    }

    memcpy(surface->pixels, pixels.data(), surface->pitch * surface->h);
    SDL_UnlockSurface(surface);

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_RenderCopy(renderer, texture, &target, &target);
    SDL_DestroyTexture(texture);
}
