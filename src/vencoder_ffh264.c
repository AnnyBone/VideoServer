#include <vencoder_ffh264.h>

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

vencoder_ffh264_t* encoder_ffh264_new (int w, int h, int fps)
{
    vencoder_ffh264_t* self =
        (vencoder_ffh264_t*)malloc (sizeof (vencoder_ffh264_t));

    if (self) {
        int rc;

        memset (self, 0, sizeof (vencoder_ffh264_t));
        self->w = w;
        self->h = h;

        int codec_id = AV_CODEC_ID_H264;

        // get h264 codec by id
        self->codec = avcodec_find_encoder (codec_id);
        if (!self->codec) {
            fprintf (stderr, "codec not found, codec_id=%d\n", codec_id);
            encoder_ffh264_destroy (&self);
            return 0;
        }

        // define encoding settings using codec context
        self->ctx = avcodec_alloc_context3 (self->codec);
        self->ctx->bit_rate = 500000;
        self->ctx->width = w;
        self->ctx->height = h;
        self->ctx->time_base = (AVRational) {1,fps};
        self->ctx->pix_fmt = AV_PIX_FMT_YUV420P;

        av_opt_set (self->ctx->priv_data, "preset", "fast", 0);
        av_opt_set (self->ctx->priv_data, "tune", "zerolatency", 0);

        // open the codec
        avcodec_open2 (self->ctx, self->codec, 0);

        // avframe
        self->frame = av_frame_alloc ();
        self->frame->format = self->ctx->pix_fmt;
        self->frame->width = self->ctx->width;
        self->frame->height = self->ctx->height;

        // allocate image buffer
        rc = av_image_alloc (self->frame->data, self->frame->linesize,
                self->ctx->width, self->ctx->height,
                self->ctx->pix_fmt, 32);
        if (rc < 0) {
            fprintf (stderr, "failed to allocate image buffer\n");
            encoder_ffh264_destroy (&self);
            return 0;
        }

        // swscontext
        self->sws = sws_getContext (self->w, self->h, AV_PIX_FMT_RGB32,
                self->w, self->h, AV_PIX_FMT_YUV420P,
                SWS_BILINEAR, 0, 0, 0);

        if (!self->sws) {
            fprintf (stderr, "failed sws context creation\n");
            encoder_ffh264_destroy (&self);
            return 0;
        }
    }

    return self;
}

void encoder_ffh264_destroy (vencoder_ffh264_t** pself)
{
    assert (pself);

    vencoder_ffh264_t* self = *pself;
    if (self) {
        avcodec_close (self->ctx);
        av_freep (&self->ctx);
        av_freep (&self->frame->data[0]);
        av_frame_free (&self->frame);
        av_packet_unref (&self->pkt);
        sws_freeContext (self->sws);
    }
}

int encoder_ffh264_encode (vencoder_ffh264_t* self, void* buffer_rgba,
        int frame_number)
{
    assert (self);

    int rc;
    int has_output;

    // clear prev. packet
    av_packet_unref (&self->pkt);

    if(buffer_rgba) {
        // convert rgba --> i420
        int linesize_rgba = self->w * 4;
        const uint8_t* src_slice[] = { buffer_rgba };
        const int src_stride[] = { linesize_rgba };
        sws_scale (self->sws, src_slice, src_stride, 0,
                self->h, self->frame->data, self->frame->linesize);

        // clear packet
        av_init_packet (&self->pkt);
        self->pkt.data = 0;
        self->pkt.size = 0;

        // set pts
        self->frame->pts = frame_number;

        // encode
        rc = avcodec_encode_video2 (self->ctx, &self->pkt, self->frame,
            &has_output);
    }
    else {
        rc = avcodec_encode_video2 (self->ctx, &self->pkt, 0,
            &has_output);
    }

    if (rc < 0) {
        fprintf (stderr, "failed to encode a frame\n");
        return -1;
    }


    return has_output? self->pkt.size : 0;
}

void* encoder_ffh264_frame (vencoder_ffh264_t* self)
{
    assert (self);
    return self->pkt.data;
}
