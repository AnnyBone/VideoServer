#include <vencoder_x264.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <libyuv.h>

vencoder_x264_t* encoder_x264_new (int w, int h, int fps)
{
    vencoder_x264_t* self = (vencoder_x264_t*) malloc (sizeof (vencoder_x264_t));
    if (self) {
        memset (self, 0, sizeof (vencoder_x264_t));

        self->w = w;
        self->h = h;
        fprintf (stdout, "size=%dx%d\n", self->w, self->h);

        if (x264_param_default_preset (&self->param, "medium", "zerolatency") < 0) {
            encoder_x264_destroy (&self);
            return 0;
        }

        self->param.i_csp = X264_CSP_I420;
        self->param.i_width = w;
        self->param.i_height = h;
        self->param.i_fps_num = fps;
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

        fprintf (stdout, "max delayed frames=%d\n",
            x264_encoder_maximum_delayed_frames (self->handle));
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

int encoder_x264_encode (vencoder_x264_t* self, void* buffer_rgba,
    int frame_number)
{
    assert (self);

    if (buffer_rgba) {
        int w2 = (self->w+1)/2;
        if (ARGBToI420 (buffer_rgba, self->w*4,
            self->pic.img.plane [0], self->w,
            self->pic.img.plane [1], w2,
            self->pic.img.plane [2], w2,
            self->w, self->h) < 0)
            return -1;

        self->pic.i_pts = frame_number;
        return x264_encoder_encode (self->handle, &self->nal,
            &self->inal, &self->pic, &self->pic_out);
    }
    else {
        return x264_encoder_encode (self->handle, &self->nal,
            &self->inal, 0, &self->pic_out);
    }


}

void* encoder_x264_frame (vencoder_x264_t* self)
{
    assert (self);
    if (!self->nal)
        return 0;

    return self->nal->p_payload;
}

bool encoder_x264_has_delayed_frames (vencoder_x264_t* self)
{
    assert (self);
    return x264_encoder_delayed_frames (self->handle);
}

