#include <grabber.h>

#include <assert.h>
#include <stdlib.h>

int grabber_init (grabber_t* self, int x, int y, int width, int height)
{
    assert (self);

    self->x = x;
    self->y = y;
    self->width = width;
    self->height = height;

    self->window = GetDesktopWindow ();
    self->window_dc = GetDC (self->window);
    self->memory_dc = CreateCompatibleDC (self->window_dc);
    self->bitmap = CreateCompatibleBitmap (self->window_dc,
		self->width, self->height);

    ZeroMemory (&self->bitmap_info, sizeof (BITMAPINFOHEADER));
    self->bitmap_info.biSize = sizeof (BITMAPINFOHEADER);
    self->bitmap_info.biPlanes = 1;
    self->bitmap_info.biBitCount = 32;
    self->bitmap_info.biWidth = width;
    self->bitmap_info.biHeight = -height;
    self->bitmap_info.biCompression = BI_RGB;
    self->bitmap_info.biSizeImage = 0;

    self->buffer = malloc (self->width * self->height * 4);
    return 0;
}

int grabber_close (grabber_t* self)
{
    assert (self);

    ReleaseDC (self->window, self->window_dc);
    DeleteDC (self->memory_dc);
    DeleteObject (self->bitmap);

    free (self->buffer);
    self->buffer = 0;

    return 0;
}

int grabber_buffer_size (grabber_t* self)
{
    assert (self);

    return self->width * self->height * 4;
}

void* grabber_capture (grabber_t* self)
{
    assert (self);

    // grab screen area
    SelectObject (self->memory_dc, self->bitmap);
    BitBlt (self->memory_dc, 0, 0, self->width, self->height,
            self->window_dc, self->x, self->y, SRCCOPY);

    GetDIBits (self->memory_dc, self->bitmap, 0, self->height, self->buffer,
            (BITMAPINFO*)&self->bitmap_info, DIB_RGB_COLORS);

	return self->buffer;
}

