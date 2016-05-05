#ifndef VDISPLAY_H
#define VDISPLAY_H

#define SDL_MAIN_HANDLED
#include <SDL.h>

struct vdisplay_s
{
    SDL_Window* screen;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Rect rect;
};

typedef struct vdisplay_s vdisplay_t;

vdisplay_t* display_new (int w, int h);
void display_destroy (vdisplay_t** pself);

void display_update (vdisplay_t* self, void* buffer);
void display_draw (vdisplay_t* self);

#endif
