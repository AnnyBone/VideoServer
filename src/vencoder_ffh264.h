#ifndef VENCODER_FFH264
#define VENCODER_FFH264

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

struct vencoder_ffh264_s
{
    AVCodec* codec;
    AVCodecContext* ctx;
    struct SwsContext* sws;
    AVFrame* frame;
    AVPacket pkt;

    int w, h;
};

typedef struct vencoder_ffh264_s vencoder_ffh264_t;

vencoder_ffh264_t* encoder_ffh264_new (int w, int h, int fps);
void encoder_ffh264_destroy (vencoder_ffh264_t** pself);

int encoder_ffh264_encode (vencoder_ffh264_t* self, void* buffer_rgba,
        int frame_number);

void* encoder_ffh264_frame (vencoder_ffh264_t* self);

#endif
