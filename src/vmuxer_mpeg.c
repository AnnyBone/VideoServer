#include <vmuxer_mpeg.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

vmuxer_mpeg_t* muxer_mpeg_new (const char* filename, int fps, AVCodec* codec)
{
    vmuxer_mpeg_t* self = (vmuxer_mpeg_t*)malloc (sizeof (vmuxer_mpeg_t));
    if (self) {
        memset (self, 0, sizeof (vmuxer_mpeg_t));

        int rc;
        self->filename = filename;
        self->codec = codec;

        // avformat output context
        rc = avformat_alloc_output_context2 (&self->fmt_ctx, 0, "mp4", self->filename);
        if (rc < 0 || !self->fmt_ctx) {
            fprintf (stderr, "failed to allocate format context\n");
            muxer_mpeg_destroy (&self);
            return 0;
        }

        self->fmt = self->fmt_ctx->oformat;

        // create stream
        // TODO provide codec
        self->stream = avformat_new_stream (self->fmt_ctx, self->codec);
        if (!self->stream) {
            fprintf (stderr, "failed to allocate stream\n");
            muxer_mpeg_destroy (&self);
            return 0;
        }

        self->stream->time_base = (AVRational){1,fps};

        // avio open
        rc = avio_open (&self->fmt_ctx->pb, self->filename, AVIO_FLAG_WRITE);
        if (rc < 0) {
            fprintf (stderr, "failed to open file '%s', message='%s'\n",
                    self->filename, av_err2str (rc));
            muxer_mpeg_destroy (&self);
            return 0;
        }

        // write stream header
        rc = avformat_write_header (self->fmt_ctx, &self->opt);
        if (rc < 0) {
            fprintf (stderr, "failed to write stream header, message='%s'\n",
                    av_err2str (rc));
            muxer_mpeg_destroy (&self);
            return 0;
        }
    }

    return self;
}

void muxer_mpeg_destroy (vmuxer_mpeg_t** pself)
{
    assert (pself);

    vmuxer_mpeg_t* self = *pself;
    if (self) {
        avio_closep (&self->fmt_ctx->pb);
        avformat_free_context (self->fmt_ctx);
        free (self);
        self = 0;
    }
}

int muxer_mpeg_write_frame (vmuxer_mpeg_t* self, void* frame, int size)
{
    assert (self);

    // TODO possibly rescale packet ts
    AVPacket pkt;

    // TODO av_packet_from_data() assumes the frame is allocated via av_malloc()
    av_packet_from_data (&pkt, (uint8_t*)frame, size);
    return av_interleaved_write_frame (self->fmt_ctx, &pkt);
}
