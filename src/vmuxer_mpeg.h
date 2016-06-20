#ifndef VMUXER_MPEG_H
#define VMUXER_MPEG_H

#include <libavformat/avformat.h>

struct vmuxer_mpeg_s
{
    AVCodecContext* codec_ctx;
    AVFormatContext* fmt_ctx;
    AVOutputFormat* fmt;
    AVStream* stream;
    AVCodec* codec;
    AVDictionary* opt;
    const char* filename;
};

typedef struct vmuxer_mpeg_s vmuxer_mpeg_t;

vmuxer_mpeg_t* muxer_mpeg_new (const char* filename, int fps, AVCodec* codec);
void muxer_mpeg_destroy (vmuxer_mpeg_t** pself);

int muxer_mpeg_write_frame (vmuxer_mpeg_t* self, void* frame, int size);


#endif

