#include <vgrabber.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

vgrabber_t* grabber_new (int x, int y, int w, int h)
{
    vgrabber_t* self = (vgrabber_t*) malloc (sizeof (vgrabber_t));
    if (self) {
        memset (self, 0, sizeof (vgrabber_t));

        self->x = x;
        self->y = y;
        self->w = w;
        self->h = h;

        self->window = GetDesktopWindow ();
        self->window_dc = GetDC (self->window);
        self->memory_dc = CreateCompatibleDC (self->window_dc);
        self->bitmap = CreateCompatibleBitmap (self->window_dc,
            self->w, self->h);

        int bpp = 4;

        ZeroMemory (&self->bitmap_info, sizeof (BITMAPINFOHEADER));
        self->bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
        self->bitmap_info.biPlanes = 1;
        self->bitmap_info.biBitCount = bpp*8;
        self->bitmap_info.biWidth = w;
        self->bitmap_info.biHeight = -h;
        self->bitmap_info.biCompression = BI_RGB;
        self->bitmap_info.biSizeImage = 0;

        self->buffer = malloc (self->w * self->h * bpp);
    }

    return self;
}

void grabber_destroy (vgrabber_t** pself)
{
    assert (pself);

    vgrabber_t* self = *pself;
    if (self) {
        ReleaseDC (self->window, self->window_dc);
        DeleteDC (self->memory_dc);
        DeleteObject (self->bitmap);

        free (self->buffer);
        free (self);
        self = 0;
    }
}

int grabber_buffer_size (vgrabber_t* self)
{
    assert (self);

    return self->w * self->h * 4;
}

void grabber_debug_info (vgrabber_t* self, const char* debug_info)
{
    assert (self);

    self->debug_info = debug_info;
}

void* grabber_capture (vgrabber_t* self)
{
    assert (self);

    // grab screen area
    SelectObject (self->memory_dc, self->bitmap);
    BitBlt (self->memory_dc, 0, 0, self->w, self->h,
            self->window_dc, self->x, self->y, SRCCOPY);

    if (self->debug_info) {
        RECT rect = { self->x, self->y, self->x + self->w, self->y + self->h };
        DrawText (self->memory_dc, self->debug_info, -1, &rect, DT_TOP|DT_RIGHT|DT_NOCLIP);
    }

    GetDIBits (self->memory_dc, self->bitmap, 0, self->h, self->buffer,
            (BITMAPINFO*)&self->bitmap_info, DIB_RGB_COLORS);

    return self->buffer;
}

