/************************************************************
 *   Author : German Mundinger
 *   Date   : 2023
 ************************************************************/

#include "hdmi_speakers.h"

#include <stdbool.h>

#include <alsa/asoundlib.h>

#include "std_error/std_error.h"


#define MALLOC_ERROR_TEXT   "Malloc error"
#define PCM_ERROR_TEXT      "Pcm error"

#define PCM_NAME "plughw:0,0"


int hdmi_speakers_init (hdmi_speakers_t * const self, hdmi_speakers_config_t const * const init_config, std_error_t * const error)
{
    assert(self != NULL);
    assert(init_config != NULL);

    self->config = *init_config;

    int exit_code = snd_pcm_open(&self->pcm_handle, PCM_NAME, SND_PCM_STREAM_PLAYBACK, 0);

    if (exit_code != 0)
    {
        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);

    snd_pcm_hw_params_any(self->pcm_handle, hw_params);

    exit_code = snd_pcm_hw_params_set_access(self->pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    exit_code = snd_pcm_hw_params_set_format(self->pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    exit_code = snd_pcm_hw_params_set_channels(self->pcm_handle, hw_params, self->config.channels);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    exit_code = snd_pcm_hw_params_set_rate_near(self->pcm_handle, hw_params, &self->config.rate_Hz, 0);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    exit_code = snd_pcm_hw_params(self->pcm_handle, hw_params);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    snd_pcm_uframes_t frames;
    exit_code = snd_pcm_hw_params_get_period_size(hw_params, &frames, 0);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    exit_code = snd_pcm_hw_params_get_period_time(hw_params, &self->period_us, 0);

    if (exit_code != 0)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, exit_code, snd_strerror(exit_code), __FILE__, __LINE__);

        return STD_FAILURE;
    }

    self->frames        = frames;
    self->buffer_size   = self->frames * self->config.channels * 2U;    // 2 -> sample size

    self->buffer = (char*)malloc(self->buffer_size * sizeof(char));

    if (self->buffer == NULL)
    {
        snd_pcm_close(self->pcm_handle);

        std_error_catch_custom(error, STD_FAILURE, MALLOC_ERROR_TEXT, __FILE__, __LINE__);

        return STD_FAILURE;
    }

    return STD_SUCCESS;
}

void hdmi_speakers_deinit (hdmi_speakers_t * const self)
{
    assert(self != NULL);

    free(self->buffer);

	snd_pcm_drain(self->pcm_handle);
	snd_pcm_close(self->pcm_handle);

    return;
}

int hdmi_speakers_play_file (hdmi_speakers_t * const self, const char *pathname, std_error_t * const error)
{
    assert(self != NULL);
    assert(pathname != NULL);

    int exit_code = STD_SUCCESS;

    int audio_fd = open(pathname, O_RDONLY);

    if (audio_fd != (-1))
    {
        while (true)
        {
            const ssize_t byte_count = read(audio_fd, self->buffer, self->buffer_size);

            if (byte_count == 0)
            {
                break;
            }

            if (byte_count == (-1))
            {
                std_error_catch_errno(error, __FILE__, __LINE__);

                exit_code = STD_FAILURE;

                break;
            }

            const snd_pcm_sframes_t frame_count = snd_pcm_writei(self->pcm_handle, self->buffer, self->frames);

            if (frame_count == (-EPIPE))
            {
                std_error_catch_custom(error, (-EPIPE), strerror((-EPIPE)), __FILE__, __LINE__);

                exit_code = STD_FAILURE;

                break;
            }
            else if (frame_count < 0)
            {
                std_error_catch_custom(error, frame_count, snd_strerror((int)frame_count), __FILE__, __LINE__);

                exit_code = STD_FAILURE;

                break;
            }
        }

        close(audio_fd);
    }
    else
    {
        std_error_catch_errno(error, __FILE__, __LINE__);

        exit_code = STD_FAILURE;
    }
    return exit_code;
}