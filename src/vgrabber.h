#ifndef VGRABBER_H
#define VGRABBER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct vgrabber_s
{
    HWND window;
    HDC window_dc, memory_dc;
    HBITMAP bitmap;
    BITMAPINFOHEADER bitmap_info;

    int x, y, w, h;
    void* buffer;

    const char* embed_str;
};

typedef struct vgrabber_s vgrabber_t;

vgrabber_t* grabber_new (int x, int y, int w, int h);
void grabber_destroy (vgrabber_t** pself);

// embed arbitrary string to the grabbed frame
void grabber_embed_str (vgrabber_t* self, const char* str);

int grabber_buffer_size (vgrabber_t* self);
void* grabber_capture (vgrabber_t* self);

#endif
