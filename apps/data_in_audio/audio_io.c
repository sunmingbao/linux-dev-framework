/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include "audio_io.h"

static snd_pcm_t *playback_handle = NULL;
static snd_pcm_t *capture_handle = NULL;

static int set_params(snd_pcm_t *handle) {
    snd_pcm_hw_params_t *params;
    int err;
    unsigned int rate = SAMPLE_RATE;

    snd_pcm_hw_params_alloca(&params);
    if ((err = snd_pcm_hw_params_any(handle, params)) < 0) return err;
    if ((err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) return err;
    if ((err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE)) < 0) return err;
    if ((err = snd_pcm_hw_params_set_channels(handle, params, CHANNELS)) < 0) return err;
    if ((err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0)) < 0) return err;
    if ((err = snd_pcm_hw_params(handle, params)) < 0) return err;

    return 0;
}

int audio_init_playback(const char *device) {
    int err;
    if ((err = snd_pcm_open(&playback_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) return err;
    return set_params(playback_handle);
}

int audio_init_capture(const char *device) {
    int err;
    if ((err = snd_pcm_open(&capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) return err;
    return set_params(capture_handle);
}

int audio_play(int16_t *buffer, size_t frames) {
    snd_pcm_sframes_t total = 0;
    while (total < frames) {
        snd_pcm_sframes_t err = snd_pcm_writei(playback_handle, buffer + total, frames - total);
        if (err < 0) {
            if (err == -EPIPE) {
                snd_pcm_prepare(playback_handle);
                continue;
            }
            return (int)err;
        }
        total += err;
    }
    return (int)total;
}

int audio_record(int16_t *buffer, size_t frames) {
    snd_pcm_sframes_t total = 0;
    while (total < frames) {
        snd_pcm_sframes_t err = snd_pcm_readi(capture_handle, buffer + total, frames - total);
        if (err < 0) {
            if (err == -EPIPE) {
                snd_pcm_prepare(capture_handle);
                continue;
            }
            return (int)err;
        }
        total += err;
    }
    return (int)total;
}

void audio_close_playback(void) {
    if (playback_handle) {
        snd_pcm_close(playback_handle);
        playback_handle = NULL;
    }
}

void audio_close_capture(void) {
    if (capture_handle) {
        snd_pcm_close(capture_handle);
        capture_handle = NULL;
    }
}
