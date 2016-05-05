#ifndef DISPLAY_H
#define DISPLAY_H

#define SDL_MAIN_HANDLED
#include <SDL.h>

struct display_s
{
    SDL_Window* screen;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect rect;
};

typedef struct display_s display_t;

display_t* display_new (int w, int h);
void display_destroy (display_t* self);

void display_update (display_t* self, void* buffer);

#endif
