/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef FRAME_BUFFER_H_
#define FRAME_BUFFER_H_

#include <stddef.h>

typedef struct frame_buffer frame_buffer_t;
typedef struct std_error std_error_t;

typedef struct frame_buffer_data
{
    int width, height;
    unsigned char *buffer;

} frame_buffer_data_t;

#ifdef __cplusplus
extern "C" {
#endif

int frame_buffer_open (frame_buffer_t * const self, const char *frame_buffer_name, std_error_t * const error);
int frame_buffer_close (frame_buffer_t * const self, std_error_t * const error);

void frame_buffer_get_data (frame_buffer_t const * const self, frame_buffer_data_t * const data);

#ifdef __cplusplus
}
#endif



// Private
#include <linux/fb.h>

typedef struct frame_buffer
{
    int file_descriptor;

    struct fb_var_screeninfo var_info;

    unsigned char *buffer;
    size_t buffer_size;

} frame_buffer_t;

#endif // FRAME_BUFFER_H_
