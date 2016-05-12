#ifndef VENCODER_VPX_H
#define VENCODER_VPX_H

#include <vpx/vpx_encoder.h>

struct vencoder_vpx_s
{
    vpx_codec_ctx_t codec;
    vpx_codec_iface_t* codec_iface;
    vpx_codec_enc_cfg_t cfg;
    vpx_image_t raw;

    int w, h;
};

typedef struct vencoder_vpx_s vencoder_vpx_t;

vencoder_vpx_t* encoder_vpx_new (int w, int h, int fps);
void encoder_vpx_destroy (vencoder_vpx_t** pself);

int encoder_vpx_encode (vencoder_vpx_t* self, void* buffer_rgba,
    int frame_number);

#endif
