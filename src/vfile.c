#include <vfile.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

vfile_t* file_new (const char* filename, const char* mode)
{
    vfile_t* self = (vfile_t*) malloc (sizeof (vfile_t));
    if (self) {
        memset (self, 0, sizeof (vfile_t));
        self->handle = fopen (filename, mode);
    }

    return self;
}

void file_destroy (vfile_t** pself)
{
    assert (pself);
    vfile_t* self = *pself;
    if (self) {
        fclose (self->handle);
        free (self);
    }
}

int file_write (vfile_t* self, void* data, int size)
{
    assert (self);

    const uint8_t* fptr = (const uint8_t*) data;
    if (!fptr || size < 0)
        return -1;

    while (size > 0) {
        int written = (int) fwrite (fptr, 1, size, self->handle);
        if (written == 0) {
            fprintf (stderr, "fwrite() failed, error=%d\n", errno);
            return -1;
        }

        size -= written;
        fptr += written;
    }

    return 0;
}

int file_read (vfile_t* self, void* data, int size)
{
    assert (self);

    uint8_t* ptr = (uint8_t*) data;
    const uint8_t* end_ptr = ptr + size;

    while (ptr < end_ptr && !feof (self->handle)) {
        int read_size = (int) (end_ptr - ptr);
        int res = (int) fread (ptr, 1, read_size, self->handle);
        if (res != size) {
            if (ferror (self->handle)) {
                fprintf (stderr, "fread() failed, error=%d\n", errno);
                return -1;
            }
        }

        ptr += res;
    }

    return 0;
}
