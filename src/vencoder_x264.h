#ifndef VENCODER_X264
#define VENCODER_X264

#include <stdint.h>

#ifdef __POCC__
#pragma warn(disable: 2135)
#endif
#include <x264.h>


struct vencoder_x264_s
{
    x264_t* handle;
    x264_picture_t pic, pic_out;
    x264_param_t param;
    x264_nal_t* nal;

    int w, h;
    int frame;
    int inal;
};

typedef struct vencoder_x264_s vencoder_x264_t;


vencoder_x264_t* encoder_x264_new (int w, int h);
void encoder_x264_destroy (vencoder_x264_t** pself);

int encoder_x264_encode (vencoder_x264_t* self, void* buffer_yuv,
        void* buffer_h264);

#if 0
void encoder_x264_decode (vencoder_x264_t* self, void* buffer_h264,
        void* buffer_yuv);
#endif
#endif
