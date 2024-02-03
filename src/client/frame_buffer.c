/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "frame_buffer.h"

#include <stdio.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "std_error/std_error.h"


#define FILE_OPENING_ERROR_TEXT "File opening error"
#define FILE_READING_ERROR_TEXT "File reading error"
#define FILE_WRITING_ERROR_TEXT "File writing error"
#define MMAP_ERROR_TEXT         "Mmap error"

#define UNUSED(x) (void)(x)
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))


int frame_buffer_open (frame_buffer_t * const self, const char *frame_buffer_name, std_error_t * const error)
{
    assert(self != NULL);
    assert(frame_buffer_name != NULL);

    self->file_descriptor = open(frame_buffer_name, O_RDWR);

    if (self->file_descriptor == (-1))
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        return STD_FAILURE;
    }

    if (ioctl(self->file_descriptor, FBIOGET_VSCREENINFO, &self->var_info) != (-1))
    {
        self->buffer_size = self->var_info.xres * self->var_info.yres * self->var_info.bits_per_pixel / 8U;
        
        self->buffer = (unsigned char*)mmap(NULL, self->buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, self->file_descriptor, 0);

        if (self->buffer == MAP_FAILED)
        {
            std_error_catch_errno(error, __FILE__, __LINE__);

            close(self->file_descriptor);

            return STD_FAILURE;
        }
    }
    else
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        close(self->file_descriptor);

        return STD_FAILURE;
    }

    return STD_SUCCESS;
}

int frame_buffer_close (frame_buffer_t * const self, std_error_t * const error)
{
    assert(self != NULL);

    if (munmap(self->buffer, self->buffer_size) != 0)
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        close(self->file_descriptor);

        return STD_FAILURE;
    }

    if (close(self->file_descriptor) != 0)
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        return STD_FAILURE;
    }

    return STD_SUCCESS;
}

void frame_buffer_get_data (frame_buffer_t const * const self, frame_buffer_data_t * const data)
{
    assert(self != NULL);
    assert(data != NULL);

    data->width     = (int)self->var_info.xres;
    data->height    = (int)self->var_info.yres;

    data->buffer = self->buffer;

    return;
}
