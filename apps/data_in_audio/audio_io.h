/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef AUDIO_IO_H
#define AUDIO_IO_H

#include <stdint.h>
#include <stddef.h>

#define SAMPLE_RATE 44100
#define CHANNELS 1

int audio_init_playback(const char *device);
int audio_init_capture(const char *device);
int audio_play(int16_t *buffer, size_t frames);
int audio_record(int16_t *buffer, size_t frames);
void audio_close_playback(void);
void audio_close_capture(void);

#endif /* AUDIO_IO_H */
