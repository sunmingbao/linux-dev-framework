# Feature: Data In Audio (Acoustic Data Transfer)

## Overview
The `data_in_audio` application enables file transfer between two Linux hosts using sound waves. It operates in two modes: **Server (Sender)** and **Client (Receiver)**.

## Requirements
- **Server Mode**: Converts a file to audio, plays a 60-second preamble, then transmits data through speakers.
- **Client Mode**: Records audio via microphone, synchronizes using the preamble, and reconstructs the original file.
- **Environment**: Linux with ALSA (Advanced Linux Sound Architecture).

## Technical Approach

### 1. Physical Layer (Modulation)
- **Modulation Scheme**: Binary Frequency Shift Keying (BFSK).
  - **Mark (1)**: 2200 Hz
  - **Space (0)**: 1200 Hz
- **Sample Rate**: 44,100 Hz, 16-bit Mono.
- **Baud Rate**: 300 - 1200 bps (to be tuned for reliability).
- **Preamble**: A constant 1000 Hz tone played for 60 seconds to allow the client to detect the signal and stabilize AGC.

### 2. Data Link Layer (Framing)
To ensure reliable transfer, data will be sent in packets:
- `[Preamble (60s)]` (Only at start)
- `[Sync Word (0xAA55)]` (To mark start of data)
- `[Packet Header: Length (2 bytes)]`
- `[Payload (N bytes)]`
- `[Checksum: CRC16 (2 bytes)]`

### 3. Software Components
The app will be located in `apps/data_in_audio/` and consist of:

#### A. Audio Engine (`audio_io.c/h`)
- Uses ALSA (`libasound`) for PCM playback and capture.
- Provides abstraction for `play_samples()` and `record_samples()`.

#### B. Modulator (`modulator.c/h`)
- Converts a byte stream into frequency-shifted sine wave samples.
- Handles preamble generation.

#### C. Demodulator (`demodulator.c/h`)
- Analyzes incoming audio samples.
- **Preamble Detection**: Uses Magnitude detection at 1000 Hz.
- **Bit Recovery**: Uses Zero-crossing detection or Goertzel algorithm/FFT to distinguish between 1200 Hz and 2200 Hz.
- **Clock Recovery**: Synchronizes bit sampling with the incoming stream.

#### D. Application Logic (`main.c`)
- CLI Argument Parsing: `-m [server|client] -f [filename]`.
- State Machine:
  - **Server**: IDLE -> PREAMBLE -> SEND_DATA -> DONE.
  - **Client**: IDLE -> LISTEN_PREAMBLE -> SYNC -> RECEIVE_DATA -> DONE.

## Implementation Plan
1. **Setup**: Create directory structure and basic `main.c`.
2. **ALSA Integration**: Implement basic tone playback and capture to verify hardware access.
3. **Modulator**: Implement BFSK tone generation for bytes.
4. **Demodulator**: Implement frequency detection and bit recovery.
5. **Framing**: Add sync words and checksums.
6. **Validation**: Test file transfer between two processes (loopback) and then two physical devices.

## Dependencies
- `libasound2-dev` (ALSA development headers).
- Project utilities from `libs/app_utils` (`log.h`, `io_utils.h`).
