/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#ifndef HDMI_SPEAKERS_H_
#define HDMI_SPEAKERS_H_

#include <stddef.h>

typedef struct hdmi_speakers hdmi_speakers_t;
typedef struct std_error std_error_t;

typedef struct hdmi_speakers_config
{
    unsigned int channels;
    unsigned int rate_Hz;

} hdmi_speakers_config_t;

#ifdef __cplusplus
extern "C" {
#endif

int hdmi_speakers_init (hdmi_speakers_t * const self, hdmi_speakers_config_t const * const init_config, std_error_t * const error);
void hdmi_speakers_deinit (hdmi_speakers_t * const self);

int hdmi_speakers_play_file (hdmi_speakers_t * const self, const char *pathname, std_error_t * const error);

#ifdef __cplusplus
}
#endif



// Private
typedef struct _snd_pcm snd_pcm_t;

typedef struct hdmi_speakers
{
    hdmi_speakers_config_t config;

    snd_pcm_t *pcm_handle;
    unsigned long int frames;
    unsigned int period_us;

    char *buffer;
    size_t buffer_size;

} hdmi_speakers_t;

#endif // HDMI_SPEAKERS_H_
