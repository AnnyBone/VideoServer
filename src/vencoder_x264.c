#include <vencoder_x264.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

vencoder_x264_t* encoder_x264_new (int w, int h)
{
    vencoder_x264_t* self = (vencoder_x264_t*) malloc (sizeof (vencoder_x264_t));
    if (self) {
        memset (self, 0, sizeof (vencoder_x264_t));
        self->w = w;
        self->h = h;

        if (x264_param_default_preset (&self->param, "medium", 0) < 0) {
            encoder_x264_destroy (&self);
            return 0;
        }

        self->param.i_csp = X264_CSP_I420;
        self->param.i_width = w;
        self->param.i_height = h;
        self->param.b_vfr_input = 0;
        self->param.b_repeat_headers = 1;
        self->param.b_annexb = 1;

        if (x264_param_apply_profile (&self->param, "high") < 0) {
            encoder_x264_destroy (&self);
            return 0;
        }

        if (x264_picture_alloc (&self->pic, self->param.i_csp,
                    self->param.i_width, self->param.i_height) < 0) {
            encoder_x264_destroy (&self);
            return 0;
        }

        self->handle = x264_encoder_open (&self->param);
        if (!self->handle) {
            encoder_x264_destroy (&self);
            return 0;
        }
    }
    return self;
}

void encoder_x264_destroy (vencoder_x264_t** pself)
{
    assert (pself);

    vencoder_x264_t* self = *pself;
    if (self) {
        if (self->handle)
            x264_encoder_close (self->handle);

        x264_picture_clean (&self->pic);
    }
}

int encoder_x264_encode (vencoder_x264_t* self,
        void* buffer_yuv, void* buffer_h264)
{
    assert (self);
    int luma_size = self->w * self->h;
    int chroma_size = luma_size/4;

    uint8_t* ptr = (uint8_t*)buffer_yuv;
    memcpy (&self->pic.img.plane [0], ptr, luma_size);
    memcpy (&self->pic.img.plane [1], ptr+luma_size, chroma_size);
    memcpy (&self->pic.img.plane [2], ptr+luma_size+chroma_size, chroma_size);

    self->pic.i_pts = self->frame;

    // TODO consider manage externally
    ++self->frame;

    int frame_size = x264_encoder_encode (self->handle, &self->nal, &self->inal,
            &self->pic, &self->pic_out);

    fprintf (stdout, "frame_size=%d\n", frame_size);

    memcpy (buffer_h264, &self->nal->p_payload, frame_size);
    return frame_size;
}
