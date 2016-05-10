#ifndef VFILE_H
#define VFILE_H

#include <stdio.h>

struct vfile_s
{
    FILE* handle;
};

typedef struct vfile_s vfile_t;

vfile_t* file_new (const char* filename, const char* mode);
void file_destroy (vfile_t** pself);

int file_write (vfile_t* self, void* data, int size);
int file_read (vfile_t* self, void* data, int size);

#endif
