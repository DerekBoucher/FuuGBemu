#ifndef PPU_H
#define PPU_H

#include "Defines.h"
#include "Memory.h"

#include <SDL.h>
#include <wx/wx.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <vector>

class PPU
{

public:
    PPU(wxWindow *, Memory *);
    PPU(PPU &) = delete;
    ~PPU();
    void RenderScreen();
    void UpdateGraphics(int);

private:
    struct sprite
    {
        uBYTE yPos;
        uBYTE xPos;
        uBYTE patternNumber;
        uBYTE attributes;
    };

    SDL_Window *sdlWindow;
    SDL_Renderer *renderer;
    uBYTE pixelData[NATIVE_SIZE_X][NATIVE_SIZE_Y];
    uBYTE LCDC;
    uBYTE STAT;
    Memory *memoryRef;
    int currentScanline;
    int scanlineCounter;

    void DrawScanline();
    void RenderPixel(int, int, uBYTE, uBYTE, uBYTE);
    void RenderTiles();
    void RenderWindow();
    void RenderSprites();
    void SetLCDStatus();
    sprite *ProcessSprites();
    uBYTE GetStat();
    uBYTE GetLCDC();
};

#endif