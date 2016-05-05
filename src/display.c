#include <display.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

display_t* display_new (int w, int h)
{
    display_t* self = (display_t*) malloc (sizeof (display_t));
    if (self) {
        memset (self, 0, sizeof (display_t));

        self->screen = SDL_CreateWindow("videosrv",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0);
        if (!self->screen) {
            fprintf (stderr, "SDL_CreateWindow() failed, message: %s\n",
                    SDL_GetError ());
            display_destroy (&self);
            return 0;
        }

        self->renderer = SDL_CreateRenderer (self->screen, -1, 0);
        if (!self->renderer) {
            fprintf (stderr, "SDL_CreateRenderer() failed, message: %s\n",
                    SDL_GetError ());
            display_destroy (&self);
            return 0;
        }

        self->texture = SDL_CreateTexture (self->renderer,
                SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
        if (!self->texture) {
            fprintf (stderr, "SDL_CreateTexture() failed, message: %s\n",
                    SDL_GetError ());
            display_destroy (&self);
            return 0;
        }

        self->rect.x = 0;
        self->rect.y = 0;
        self->rect.w = w;
        self->rect.h = h;
    }

    return self;
}

void display_destroy (display_t** pself)
{
    assert (pself);

    display_t* self = *pself;
    if (self) {
        if (self->texture)
            SDL_DestroyTexture (self->texture);

        if (self->renderer)
            SDL_DestroyRenderer (self->renderer);

        if (self->screen)
            SDL_DestroyWindow (self->screen);

        free (self);
        self = 0;
    }
}

void display_update (display_t* self, void* buffer)
{
    assert (self);

    int bpp = 4;
    int line_size = self->rect.w * bpp;
    SDL_UpdateTexture (self->texture, &self->rect, buffer, line_size);
}

void display_draw (display_t* self)
{
    assert (self);

    SDL_RenderCopy (self->renderer, self->texture, 0, 0);
    SDL_RenderPresent (self->renderer);
}
