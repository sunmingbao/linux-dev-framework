/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <math.h>
#include "modulator.h"
#include "audio_io.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void generate_tone(int16_t *buffer, int freq, size_t frames, double *phase) {
    double phase_inc = (2.0 * M_PI * freq) / SAMPLE_RATE;
    for (size_t i = 0; i < frames; i++) {
        *phase += phase_inc;
        if (*phase >= 2.0 * M_PI) *phase -= 2.0 * M_PI;
        buffer[i] = (int16_t)(32767.0 * sin(*phase));
    }
}

void modulate_byte(uint8_t byte, int16_t *buffer, double *phase) {
    size_t samples_per_bit = SAMPLE_RATE / BAUD_RATE;
    // Simple BFSK modulation: 1 start bit, 8 data bits, 1 stop bit
    // Start bit (Space)
    generate_tone(buffer, FREQ_SPACE, samples_per_bit, phase);
    buffer += samples_per_bit;

    // Data bits
    for (int i = 0; i < 8; i++) {
        int freq = (byte & (1 << i)) ? FREQ_MARK : FREQ_SPACE;
        generate_tone(buffer, freq, samples_per_bit, phase);
        buffer += samples_per_bit;
    }

    // Stop bit (Mark)
    generate_tone(buffer, FREQ_MARK, samples_per_bit, phase);
}
