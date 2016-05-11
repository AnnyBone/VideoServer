#ifndef VENCODER_X264
#define VENCODER_X264

#include <stdint.h>
#include <stdbool.h>
#include <x264.h>

struct vencoder_x264_s
{
    x264_t* handle;
    x264_picture_t pic, pic_out;
    x264_param_t param;
    x264_nal_t* nal;

    int w, h;
    int inal;
};

typedef struct vencoder_x264_s vencoder_x264_t;


vencoder_x264_t* encoder_x264_new (int w, int h, int fps);
void encoder_x264_destroy (vencoder_x264_t** pself);

int encoder_x264_encode (vencoder_x264_t* self, void* buffer_rgba, int frame_number);
void* encoder_x264_frame (vencoder_x264_t* self);

bool encoder_x264_has_delayed_frames (vencoder_x264_t* self);

#endif
