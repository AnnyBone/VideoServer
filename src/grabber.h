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

	int x, y, width, height;
    void* buffer;
};

typedef struct grabber_s grabber_t;

int grabber_init (grabber_t* self, int x, int y, int width, int height);
int grabber_close (grabber_t* self);

int grabber_buffer_size (grabber_t* self);
void* grabber_capture (grabber_t* self);

#endif
