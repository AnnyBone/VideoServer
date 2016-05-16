#include <vencoder_vpx.h>

#include <vpx/vpx_codec.h>
#include <vpx/vp8cx.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libyuv.h>

vencoder_vpx_t* encoder_vpx_new (int w, int h, int fps)
{
    vencoder_vpx_t* self = (vencoder_vpx_t*)malloc (sizeof (vencoder_vpx_t));
    if (self) {
        memset (self, 0, sizeof (vencoder_vpx_t));

        self->w = w;
        self->h = h;

        self->codec_iface = vpx_codec_vp9_cx ();
        fprintf (stdout, "using %s\n",
                vpx_codec_iface_name (self->codec_iface));

        if (!vpx_img_alloc (&self->raw, VPX_IMG_FMT_I420, w, h, 1)) {
            fprintf (stderr, "failed to allocate image\n");
            encoder_vpx_destroy (&self);
            return 0;
        }

        vpx_codec_err_t rc;
        rc = vpx_codec_enc_config_default (self->codec_iface, &self->cfg, 0);
        if (rc) {
            fprintf (stderr, "failed to get default codec config\n");
            encoder_vpx_destroy (&self);
            return 0;
        }

        self->cfg.g_w = self->w;
        self->cfg.g_h = self->h;
        self->cfg.g_timebase.num = 1;
        self->cfg.g_timebase.den = fps;
        self->cfg.g_lag_in_frames = 0;
        self->cfg.rc_end_usage = VPX_CBR;
        self->cfg.rc_target_bitrate = 500;

        rc = vpx_codec_enc_init (&self->codec, self->codec_iface,
                &self->cfg, 0);
        if (rc) {
            fprintf (stderr, "failed to initialize encoder\n");
            encoder_vpx_destroy (&self);
            return 0;
        }

        vpx_codec_control(&self->codec, VP8E_SET_CPUUSED, 5);
    }

    return self;
}

void encoder_vpx_destroy (vencoder_vpx_t** pself)
{
    assert (pself);
    vencoder_vpx_t* self = *pself;
    if (self) {
        vpx_img_free (&self->raw);
        vpx_codec_destroy (&self->codec);
        free (self);
        self = 0;
    }
}

int encoder_vpx_encode (vencoder_vpx_t* self, void* buffer_rgba,
        int frame_number)
{
    assert (self);

    int w2 = (self->w+1)/2;
    if (ARGBToI420 (buffer_rgba, self->w*4,
        self->raw.planes [0], self->w,
        self->raw.planes [1], w2,
        self->raw.planes [2], w2,
        self->w, self->h) < 0)
        return -1;

    int flags = 0;

    vpx_codec_err_t rc;
    rc = vpx_codec_encode (&self->codec, &self->raw, frame_number, 1, flags,
            VPX_DL_REALTIME);

    if (rc != VPX_CODEC_OK)
        return -1;

    // TODO
    return 0;
}
