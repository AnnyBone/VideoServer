#ifndef GRABBER_H
#define GRABBER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct grabber_s
{
    HWND window;
    HDC window_dc, memory_dc;
    HBITMAP bitmap;
    BITMAPINFOHEADER bitmap_info;

    int x, y, w, h;
    void* buffer;
};

typedef struct grabber_s grabber_t;

grabber_t* grabber_new (int x, int y, int w, int h);
void grabber_destroy (grabber_t** pself);

int grabber_buffer_size (grabber_t* self);
void* grabber_capture (grabber_t* self);

#endif
