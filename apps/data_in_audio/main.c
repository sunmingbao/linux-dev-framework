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
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "audio_io.h"
#include "modulator.h"
#include "demodulator.h"
#include "log.h"

#define SYNC_WORD 0xAA55

void print_usage(const char *progname) {
    printf("Usage: %s -m [server|client] -f [filename]\n", progname);
}

void server_mode(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        ErrSysLog("Could not open file: %s", filename);
        return;
    }

    if (audio_init_playback("default") < 0) {
        ErrSysLog("Failed to initialize playback");
        fclose(f);
        return;
    }

    SysLog("Starting preamble (30s)...");
    double phase = 0.0;
    int16_t *preamble_buffer = malloc(SAMPLE_RATE * sizeof(int16_t));
    for (int i = 0; i < 30; i++) {
        generate_tone(preamble_buffer, FREQ_PREAMBLE, SAMPLE_RATE, &phase);
        audio_play(preamble_buffer, SAMPLE_RATE);
    }
    free(preamble_buffer);

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    SysLog("Transmitting file data (%ld bytes)...", file_size);
    size_t samples_per_byte = (SAMPLE_RATE / BAUD_RATE) * 10; // 1 start, 8 data, 1 stop
    int16_t *data_buffer = malloc(samples_per_byte * sizeof(int16_t));
    
    // Send sync word
    modulate_byte((SYNC_WORD >> 8) & 0xFF, data_buffer, &phase);
    if (audio_play(data_buffer, samples_per_byte) < 0) {
        ErrSysLog("Failed to play sync word high");
    }
    modulate_byte(SYNC_WORD & 0xFF, data_buffer, &phase);
    if (audio_play(data_buffer, samples_per_byte) < 0) {
        ErrSysLog("Failed to play sync word low");
    }

    uint8_t byte;
    long sent = 0;
    while (fread(&byte, 1, 1, f) == 1) {
        modulate_byte(byte, data_buffer, &phase);
        if (audio_play(data_buffer, samples_per_byte) < 0) {
            ErrSysLog("Failed to play data byte at offset %ld", sent);
            break;
        }
        sent++;
        if (sent % 1000 == 0) SysLog("Sent %ld/%ld bytes...", sent, file_size);
    }

    free(data_buffer);
    audio_close_playback();
    fclose(f);
    SysLog("Transmission complete.");
}

void client_mode(const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        ErrSysLog("Could not open file: %s", filename);
        return;
    }

    if (audio_init_capture("default") < 0) {
        ErrSysLog("Failed to initialize capture");
        fclose(f);
        return;
    }

    SysLog("Waiting for preamble...");
    if (wait_for_preamble() < 0) {
        ErrSysLog("Preamble detection failed");
        audio_close_capture();
        fclose(f);
        return;
    }

    SysLog("Preamble detected. Waiting for Sync Word...");
    uint8_t b1, b2;
    while (1) {
        if (demodulate_byte(&b1) < 0) break;
        if (b1 == ((SYNC_WORD >> 8) & 0xFF)) {
            if (demodulate_byte(&b2) < 0) break;
            if (b2 == (SYNC_WORD & 0xFF)) break;
        }
    }

    SysLog("Sync Word detected. Receiving file data...");
    uint8_t byte;
    while (demodulate_byte(&byte) == 0) {
        fwrite(&byte, 1, 1, f);
        fflush(f);
    }

    audio_close_capture();
    fclose(f);
    SysLog("Reception complete.");
}

int main(int argc, char *argv[]) {
    char *mode = NULL;
    char *filename = NULL;
    int opt;

    while ((opt = getopt(argc, argv, "m:f:")) != -1) {
        switch (opt) {
            case 'm': mode = optarg; break;
            case 'f': filename = optarg; break;
            default: print_usage(argv[0]); return 1;
        }
    }

    if (!mode || !filename) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(mode, "server") == 0) {
        server_mode(filename);
    } else if (strcmp(mode, "client") == 0) {
        client_mode(filename);
    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
