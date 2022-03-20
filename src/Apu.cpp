#include "Apu.hpp"

bool FrequencyOverflowCheck(int shadowFrequency, int shiftAmount, bool increaseFrequency) {
    int newFrequency = shadowFrequency >> shiftAmount;

    if (increaseFrequency) {
        newFrequency = shadowFrequency + newFrequency;
    }
    else {
        newFrequency = shadowFrequency - newFrequency;
    }

    return newFrequency > 2047;
}

void AdjustChannelVolume(int& channelTimer, int& channelVolume, bool volumeDirection, int replenishPeriod) {
    if (channelTimer > 0) {
        channelTimer--;
    }

    // For clarity
    bool upwards = true;
    bool downwards = false;

    if (channelTimer == 0) {
        channelTimer = replenishPeriod;

        if ((channelVolume < 0x0F && volumeDirection == upwards) | (channelVolume > 0x00 && volumeDirection == downwards)) {
            if (volumeDirection == upwards) {
                channelVolume++;
            }
            else {
                channelVolume--;
            }
        }
    }
}

Apu::Apu() {}

Apu::Apu(Memory* memRef) {
    pa_sample_spec samplingSpec = pa_sample_spec();
    samplingSpec.format = PA_SAMPLE_S16BE;
    samplingSpec.rate = APU_SAMEPLE_RATE;
    samplingSpec.channels = 2;

    int errorCode = 0;

    audioClient = pa_simple_new(NULL,
        "FuuGBEmuAPU",
        PA_STREAM_PLAYBACK,
        NULL,
        "test",
        &samplingSpec,
        NULL,
        NULL,
        &errorCode);

    // Error checking
    if (errorCode) {
        fprintf(stderr, "error initializing audio: %s\n", pa_strerror(errorCode));
        exit(EXIT_FAILURE);
    }

    this->memRef = memRef;

    // Initialize Frequency Timer for Channel 2
    ch2FrequencyTimer = CH2_FREQUENCY_CYCLES;

    ch4FrequencyTimer = 8;

    // Initialize Frame Sequencer Timer
    frameSequencerTimer = FRAME_SEQUENCER_CYCLES;

    // Channel 2 bit position
    channel2DutyPosition = 0;

    // Frame Sequencer Step
    frameSequencerStep = 0;

    // Envelope Timers
    ch1EnvelopeTimer = 0;
    ch2EnvelopeTimer = 0;
    ch4EnvelopeTimer = 0;

    // Envelope Volumes
    ch1EnvelopeVolume = 0;
    ch2EnvelopeVolume = 0;
    ch4EnvelopeVolume = 0;

    // Sweep register
    sweepEnabled = false;
    shadowFrequency = 0;
    sweepTimer = CH1_SWEEP_CYCLES;

    ch1LengthTimer = 0;
    ch2LengthTimer = 0;
    ch3LengthTimer = 0;
    ch4LengthTimer = 0;

    ch4LFSR = 0xFFFF;

    addToBufferTimer = CPU_FREQUENCY / APU_SAMEPLE_RATE;
    currentPositionBuffer = 0;

    currentBuffer = 0;
}

Apu::~Apu() {
    if (audioClient) {
        pa_simple_flush(audioClient, NULL);
        pa_simple_free(audioClient);
    }
}

void Apu::WaitFlush() {
    int errorCode = 0;
    pa_simple_drain(audioClient, &errorCode);

    if (errorCode) {
        fprintf(stderr, "error flushing audio buffer: %s\n", pa_strerror(errorCode));
    }
}

void Apu::PlaySample() {
    int errorCode = 0;

    for (int j = 0; j < 1024; j++) {
        for (int i = 0; i < BUFFER; i++) {
            buffer[0][i] = j;
        }

        pa_simple_write(audioClient, buffer[0], BUFFER, &errorCode);
    }

    if (errorCode) {
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
    }
}

void Apu::AddSampleToBuffer(int cycles) {
    addToBufferTimer -= cycles;

    if (addToBufferTimer <= 0) {
        addToBufferTimer = CPU_FREQUENCY / APU_SAMEPLE_RATE;

        buffer[currentBuffer][currentPositionBuffer++] = 0x45;
        buffer[currentBuffer][currentPositionBuffer++] = 0x7F;

        if (currentPositionBuffer >= BUFFER) {
            FlushBuffer();
            currentPositionBuffer = 0;

            if (currentBuffer) {
                currentBuffer = 0;
            }
            else {
                currentBuffer = 1;
            }
        }
    }
}

void Apu::FlushBuffer() {
    int errorCode;
    pa_simple_write(audioClient, buffer[currentBuffer], BUFFER, &errorCode);
    if (errorCode) {
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
    }

    pa_simple_drain(audioClient, &errorCode);

    if (errorCode) {
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
    }
}

void Apu::UpdateSoundRegisters(int cycles) {

    // Handle Frequency Timer
    ch2FrequencyTimer -= cycles;

    uBYTE ch2DutyPattern = memRef->rom[NR21] >> 6;

    if (ch2FrequencyTimer <= 0) {
        int extraCycles = (ch2FrequencyTimer * -1);

        uBYTE ch2FrqLo = memRef->rom[NR23];
        uBYTE ch2FrqHi = memRef->rom[NR24] & 0b00000111;
        uWORD ch2Frq = (ch2FrqHi << 8) | ch2FrqLo;

        ch2FrequencyTimer = ((CH2_FREQUENCY_CYCLES - ch2Frq) * 4) - extraCycles;

        channel2DutyPosition = (channel2DutyPosition + 1) % 8;
    }

    // Handle Frame Sequencer
    frameSequencerTimer -= cycles;

    if (frameSequencerTimer <= 0) {
        int extraCycles = (frameSequencerTimer * -1);
        frameSequencerStep = (frameSequencerStep + 1) % 8;
        frameSequencerTimer = FRAME_SEQUENCER_CYCLES - extraCycles;
    }

    // Handle Envelope Functions
    uBYTE ch1EnvelopeRegister = memRef->rom[NR12];
    uBYTE ch2EnvelopeRegister = memRef->rom[NR22];
    uBYTE ch4EnvelopeRegister = memRef->rom[NR42];

    bool ch1TriggerEvent = memRef->rom[NR14] & (1 << 7);
    bool ch2TriggerEvent = memRef->rom[NR24] & (1 << 7);
    bool ch3TriggerEvent = memRef->rom[NR34] & (1 << 7);
    bool ch4TriggerEvent = memRef->rom[NR44] & (1 << 7);

    // Channel 1
    if (ch1TriggerEvent) {
        ch1EnvelopeTimer = ch1EnvelopeRegister & 0b00000111;
        ch1EnvelopeVolume = ((ch1EnvelopeRegister & 0b11110000) >> 4);
        int ch1LengthData = memRef->rom[NR11] & 0b00011111;

        shadowFrequency = ((memRef->rom[NR14] & 0b00000111) << 8) | (memRef->rom[NR13]);
        sweepTimer = (memRef->rom[NR10] & 0b01110000) >> 5;
        int sweepShiftAmount = memRef->rom[NR10] & 0b00000111;
        bool increaseFrequency = memRef->rom[NR10] & 0b00001000;

        if (sweepTimer == 0) {
            sweepTimer = 8;
        }

        if (sweepTimer > 0 || sweepShiftAmount > 0) {
            sweepEnabled = true;
        }
        else {
            sweepEnabled = false;
        }

        if (sweepShiftAmount > 0) {
            if (FrequencyOverflowCheck(shadowFrequency, sweepShiftAmount, increaseFrequency)) {
                sweepEnabled = false;
            }
        }

        if (ch1LengthTimer == 0) {
            ch1LengthTimer = 64 - ch1LengthData;
            // enable channel 1
        }
    }

    // Channel 2
    if (ch2TriggerEvent) {
        ch2EnvelopeTimer = ch2EnvelopeRegister & 0b00000111;
        ch2EnvelopeVolume = ((ch2EnvelopeRegister & 0b11110000) >> 4);
        int ch2LengthData = memRef->rom[NR21] & 0b00011111;

        if (ch2LengthTimer == 0) {
            ch2LengthTimer = 64 - ch2LengthData;
            // enable channel 2
        }
    }

    // Channel 3
    if (ch3TriggerEvent) {
        int ch3LengthData = memRef->rom[NR31];
        if (ch3LengthTimer == 0) {
            ch3LengthTimer = 256 - ch3LengthData;
            //enable channel 3
        }
    }

    // Channel 4
    if (ch4TriggerEvent) {
        ch4EnvelopeTimer = ch4EnvelopeRegister & 0b00000111;
        ch4EnvelopeVolume = ((ch4EnvelopeRegister & 0b11110000) >> 4);
        int ch4LengthData = memRef->rom[NR41] & 0b00011111;

        if (ch4LengthTimer == 0) {
            ch4LengthTimer = 64 - ch4LengthData;
            // enable channel 4
        }

        ch4LFSR = 0xFFFF;
    }

    // If the frame sequencer clocks a volume clock, start evaluating 
    // envelope functions for channel 1, 2 and 4. (Channel 3 has no envelope)
    if (frameSequencerStep == FRAME_SEQUENCE_VOLUME_CLOCK) {
        if (ch1EnvelopeTimer != 0) {
            bool volumeDirection = ch1EnvelopeRegister & 0b00001000;
            int replenishPeriod = ch1EnvelopeRegister & 0b00000111;
            AdjustChannelVolume(ch1EnvelopeTimer, ch1EnvelopeVolume, volumeDirection, replenishPeriod);
        }

        if (ch2EnvelopeTimer != 0) {
            bool volumeDirection = ch2EnvelopeRegister & 0b00001000;
            int replenishPeriod = ch2EnvelopeRegister & 0b00000111;
            AdjustChannelVolume(ch2EnvelopeTimer, ch2EnvelopeVolume, volumeDirection, replenishPeriod);
        }

        if (ch4EnvelopeTimer != 0) {
            bool volumeDirection = ch4EnvelopeRegister & 0b00001000;
            int replenishPeriod = ch4EnvelopeRegister & 0b00000111;
            AdjustChannelVolume(ch4EnvelopeTimer, ch4EnvelopeVolume, volumeDirection, replenishPeriod);
        }
    }

    // Handle Frequency Sweeping for channel 1
    if (frameSequencerStep == FRAME_SEQUENCE_SWEEP_CLOCK_2 || frameSequencerStep == FRAME_SEQUENCE_SWEEP_CLOCK_6) {
        if (sweepTimer > 0) {
            sweepTimer--;
        }

        if (sweepTimer == 0) {
            int sweepPeriod = (memRef->rom[NR10] & 0b01110000) >> 5;
            sweepTimer = sweepPeriod;
            if (sweepTimer == 0) {
                sweepTimer = 8;
            }

            if (sweepEnabled && sweepPeriod > 0) {
                int sweepShift = memRef->rom[NR10] & 0b00000111;
                int newFrequency = shadowFrequency >> sweepShift;
                bool increaseFrequency = memRef->rom[NR10] & 0b00001000;

                if (increaseFrequency) {
                    newFrequency = shadowFrequency + newFrequency;
                }
                else {
                    newFrequency = shadowFrequency - newFrequency;
                }

                if (newFrequency > 2047) {
                    // disable  channel 1
                }
            }
        }
    }

    // Handle length timer calculation
    if (frameSequencerStep % 2 == 0 && memRef->rom[NR24] & 0b01000000) {
        ch1LengthTimer--;
        ch2LengthTimer--;
        ch3LengthTimer--;
        ch4LengthTimer--;

        if (ch1LengthTimer == 0) {
            // disable channel 1
        }

        if (ch2LengthTimer == 0) {
            // disable channel 2
        }

        if (ch3LengthTimer == 0) {
            // disable channel 3
        }

        if (ch4LengthTimer == 0) {
            // disable channel 4
        }
    }

    // Read WAVE ram for channel 3
    uBYTE waveRAM[16];
    for (uint i = 0; i < 16; i++) {
        waveRAM[i] = memRef->rom[0xFF30 + i];
    }

    int ch3shiftAmount = ((memRef->rom[NR32] & 0b01100000) >> 5) - 1;

    // Channel 4 noise setup
    int ch4ShiftAmount = (memRef->rom[NR43] & 0b11110000) >> 4;
    int ch4WidthCounter = (memRef->rom[NR43] & 0b00001000) >> 3;
    int ch4DivisorCode = memRef->rom[NR43] & 0b00000111;

    int ch4Divisor = 0;
    switch (ch4DivisorCode) {
    case 0: ch4Divisor = 8; break;
    case 1: ch4Divisor = 16; break;
    case 2: ch4Divisor = 32; break;
    case 3: ch4Divisor = 48; break;
    case 4: ch4Divisor = 64; break;
    case 5: ch4Divisor = 80; break;
    case 6: ch4Divisor = 96; break;
    case 7: ch4Divisor = 112; break;
    default: ch4Divisor = 8; break;
    }

    ch4FrequencyTimer -= cycles;
    if (ch4FrequencyTimer <= 0) {
        ch4FrequencyTimer = ch4Divisor << ch4ShiftAmount;

        int xorResult = (ch4LFSR & 0b01) ^ ((ch4LFSR & 0b10) >> 1);
        ch4LFSR = (ch4LFSR >> 1) | (xorResult << 14);

        if (ch4WidthCounter) {
            ch4LFSR &= !(1 << 6);
            ch4LFSR |= xorResult << 6;
        }
    }

    int leftAmplitude = 0;
    int rightAmplitude = 0;

    // Calculate Amplitudes
    uBYTE ch2Wave = 0;
    switch (ch2DutyPattern) {
    case 0: ch2Wave = 0b00000001; break;
    case 1: ch2Wave = 0b00000011; break;
    case 2: ch2Wave = 0b00001111; break;
    case 3: ch2Wave = 0b11111100; break;
    default: ch2Wave = 0b00000001; break;
    }

    uBYTE ch2Amplitude = ch2Wave & (1 << channel2DutyPosition);
    uBYTE ch4Amplitude = ~ch4LFSR & 0x01;

    if (!(memRef->rom[NR52] & (1 << 0))) {
        // disable channel 1
    }

    // if (!(memRef->rom[NR52] & (1 << 1))) {
    //     ch2Amplitude = 0;
    // }

    // if (!(memRef->rom[NR52] & (1 << 2))) {
    //     // disable channel 3
    // }

    // if (!(memRef->rom[NR52] & (1 << 3))) {
    //     ch4Amplitude = 0;
    // }


    // Determine panning
    if (memRef->rom[NR51] & 0x20) {
        // TBD
    }

    // Get volume levels
    uBYTE leftVolume = memRef->rom[NR50] & 0b00000111;
    uBYTE rightVolume = (memRef->rom[NR50] & 0b01110000) >> 4;

    leftSample = ((ch2Amplitude + ch4Amplitude) / 2) * leftVolume;
    rightSample = ((ch2Amplitude + ch4Amplitude) / 2) * rightVolume;
}
