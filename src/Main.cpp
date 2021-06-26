#include <wx/app.h>
#include <SDL.h>

int main(int argc, char **argv)
{

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("Error occured during SDL2 initialization: %s\n", SDL_GetError());
        return -1;
    }

    return wxEntry(argc, argv);
}