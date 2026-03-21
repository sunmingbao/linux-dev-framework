/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef MODULATOR_H
#define MODULATOR_H

#include <stdint.h>
#include <stddef.h>

#define FREQ_MARK  2200
#define FREQ_SPACE 1200
#define FREQ_PREAMBLE 1000
#define BAUD_RATE 300

void generate_tone(int16_t *buffer, int freq, size_t frames, double *phase);
void modulate_byte(uint8_t byte, int16_t *buffer, double *phase);

#endif /* MODULATOR_H */
