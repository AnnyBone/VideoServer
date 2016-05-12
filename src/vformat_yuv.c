#include <vformat_yuv.h>
#include <stdint.h>

#include <libyuv.h>

int format_rgba_to_i420 (void* buffer_rgba, int w, int h, void* buffer_i420)
{
    if (!buffer_rgba || !buffer_i420 || w <= 0 || h <= 0)
        return -1;

    int w2 = (w+1)/2;
    int h2 = (h+1)/2;
    return ARGBToI420 (buffer_rgba, w*4,
        buffer_i420, w,
        (uint8_t*)buffer_i420 + w*h, w2,
        (uint8_t*)buffer_i420 + w*h + w2*h2, w2,
        w, h);
}
