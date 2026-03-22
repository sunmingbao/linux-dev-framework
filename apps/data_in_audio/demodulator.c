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
#include <string.h>
#include "demodulator.h"
#include "audio_io.h"
#include "modulator.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Goertzel algorithm to detect magnitude of a specific frequency
double goertzel_magnitude(int16_t *samples, int num_samples, int target_freq) {
    double s_prev = 0.0;
    double s_prev2 = 0.0;
    double coeff = 2.0 * cos(2.0 * M_PI * target_freq / SAMPLE_RATE);
    
    for (int i = 0; i < num_samples; i++) {
        double s = (double)samples[i] + coeff * s_prev - s_prev2;
        s_prev2 = s_prev;
        s_prev = s;
    }
    
    return s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2;
}

int wait_for_preamble(void) {
    size_t samples_per_check = SAMPLE_RATE / 10; // 100ms
    int16_t buffer[samples_per_check];
    
    while (1) {
        if (audio_record(buffer, samples_per_check) < 0) return -1;
        double mag = goertzel_magnitude(buffer, samples_per_check, FREQ_PREAMBLE);
        if (mag > 1e12) return 0; // Threshold for preamble detection
    }
}

int demodulate_byte(uint8_t *byte) {
    size_t samples_per_bit = SAMPLE_RATE / BAUD_RATE;
    // Use a larger buffer to accommodate skips
    int16_t buffer[samples_per_bit * 2];
    
    // 1. Wait for Start bit (Space / 1200 Hz) leading edge
    size_t check_size = samples_per_bit / 2;
    if (check_size == 0) check_size = 1;
    while (1) {
        if (audio_record(buffer, check_size) < 0) return -1;
        double mag_space = goertzel_magnitude(buffer, check_size, FREQ_SPACE);
        double mag_mark = goertzel_magnitude(buffer, check_size, FREQ_MARK);
        // Half-bit window (N=73) has good frequency resolution.
        // SPACE must be dominant over MARK and above noise threshold.
        if (mag_space > 2e8 && mag_space > mag_mark * 2) break;
    }
    
    // 2. Skip the rest of the start bit to reach the start of the first data bit.
    // We already consumed 'check_size' samples.
    audio_record(buffer, samples_per_bit - check_size);
    
    // 3. Sample 8 data bits using full-bit windows
    *byte = 0;
    for (int i = 0; i < 8; i++) {
        if (audio_record(buffer, samples_per_bit) < 0) return -1;
        double mag_mark = goertzel_magnitude(buffer, samples_per_bit, FREQ_MARK);
        double mag_space = goertzel_magnitude(buffer, samples_per_bit, FREQ_SPACE);
        if (mag_mark > mag_space) {
            *byte |= (1 << i);
        }
    }
    
    // 4. No need to skip the stop bit, the next call will wait for the next START bit.
    return 0;
}
