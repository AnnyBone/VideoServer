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
        self->bpp = 4;

        self->window = GetDesktopWindow ();
        self->window_dc = GetDC (self->window);
        self->memory_dc = CreateCompatibleDC (self->window_dc);
        self->bitmap = CreateCompatibleBitmap (self->window_dc,
            self->w, self->h);


        ZeroMemory (&self->bitmap_info, sizeof (BITMAPINFOHEADER));
        self->bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
        self->bitmap_info.biPlanes = 1;
        self->bitmap_info.biBitCount = self->bpp*8;
        self->bitmap_info.biWidth = w;
        self->bitmap_info.biHeight = -h;
        self->bitmap_info.biCompression = BI_RGB;
        self->bitmap_info.biSizeImage = 0;
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

        free (self);
        self = 0;
    }
}

int grabber_buffer_size (vgrabber_t* self)
{
    assert (self);

    return self->w * self->h * 4;
}

void grabber_embed_str (vgrabber_t* self, const char* str)
{
    assert (self);

    self->embed_str = str;
}

void grabber_capture (vgrabber_t* self, void* buffer)
{
    assert (self);

    // grab screen area
    SelectObject (self->memory_dc, self->bitmap);
    BitBlt (self->memory_dc, 0, 0, self->w, self->h,
            self->window_dc, self->x, self->y, SRCCOPY);

    if (self->embed_str) {
        RECT rect = {
            self->x,
            self->y,
            self->x + self->w,
            self->y + self->h
        };
        DrawText (self->memory_dc, self->embed_str, -1, &rect,
            DT_TOP|DT_RIGHT|DT_NOCLIP);
    }

    GetDIBits (self->memory_dc, self->bitmap, 0, self->h, buffer,
            (BITMAPINFO*)&self->bitmap_info, DIB_RGB_COLORS);
}

