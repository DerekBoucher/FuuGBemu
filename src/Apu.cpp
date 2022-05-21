#include "Apu.hpp"

Apu::Apu() {}

Apu::Apu(Memory* memRef) {
    pa_sample_spec samplingSpec = pa_sample_spec();
    samplingSpec.format = PA_SAMPLE_U8;
    samplingSpec.rate = APU_SAMPLE_RATE;
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

    addToBufferTimer = CPU_FREQUENCY / APU_SAMPLE_RATE;

    ch2FrequencyTimer = 0;
    ch2WaveDutyPointer = 0;
    currentSampleBufferPosition = 0;
}

Apu::~Apu() {
    pa_simple_free(audioClient);
}

void Apu::FlushBuffer() {
    int errorCode = 0;

    pa_simple_write(audioClient, buffer, BUFFER, &errorCode);
    if (errorCode) {
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
    }

    pa_simple_drain(audioClient, &errorCode);
    if (errorCode) {
        fprintf(stderr, "error flushing audio buffer: %s\n", pa_strerror(errorCode));
    }
}

void Apu::UpdateSoundRegisters(int cycles) {
    // First, check if it is time to add a sample to the
    // audio buffer. We only need to send samples every
    // CPU_FREQUENCY / APU_SAMPLE_RATE cycles (which equates to 87 cycles, in this case)
    addToBufferTimer -= cycles;
    if (addToBufferTimer > 0) {
        return;
    }

    // If it is time to add new samples, refresh the timer value.
    addToBufferTimer = CPU_FREQUENCY / APU_SAMPLE_RATE;

    // Channel 2
    uBYTE ch2Amplitude = ComputeChannel2Amplitude();

    // Mix the amplitudes of all channels
    uBYTE sample = ch2Amplitude / 1;

    uBYTE nr50 = memRef->rom[NR50];
    uBYTE nr51 = memRef->rom[NR51];
    uBYTE nr52 = memRef->rom[NR52];

    // If we are currently working with the right
    // stereo sample, modulo 2 returns 1
    if (currentSampleBufferPosition % 2) {
        // Verify if NR51 register disables the right output for channel 2
        if ((nr51 & (1 << 5)) == 0) {
            ch2Amplitude = 0;
        }
    }
    else { // else we are dealing with the left stereo sample

        // Verify if NR51 register disables the left output for channel 2
        if ((nr51 & (1 << 1)) == 0) {
            ch2Amplitude = 0;
        }
    }

    uBYTE volume = 0;

    // Apply Right volume modifier
    if (currentSampleBufferPosition % 2) {
        volume = (nr50 & (0b00000111));
    }
    // Apply left volume modifier
    else {
        volume = (nr50 & (0b01110000)) >> 4;
    }

    sample = sample * volume;

    // TODO: Determine what to do with NR52 bits 0-3

    // Finally, stuff the sample in the buffer,
    // and play the sound if the buffer is full
    buffer[currentSampleBufferPosition++] = sample;
    if (currentSampleBufferPosition >= BUFFER) {
        currentSampleBufferPosition = 0;

        // If bit 7 of NR52 is reset, this means that the volume
        // is completely shut off. Only flush the buffer if the master
        // switch is ON.
        if ((nr52 & (1 << 7))) {
            FlushBuffer();
        }
    }
}

uBYTE Apu::DetermineChannel2WavePattern() {
    // Wave duty identifier is found in bit 7-6 of NR21
    uBYTE waveDuty = (memRef->rom[NR21] & (0b11000000) >> 6);
    uBYTE wavePattern = 0;

    switch (waveDuty) {
    case 0: wavePattern = 0b00000001; break;
    case 1: wavePattern = 0b00000011; break;
    case 2: wavePattern = 0b00001111; break;
    case 3: wavePattern = 0b11111100; break;
    default:
#ifdef FUUGB_DEBUG
        fprintf(stderr, "[APU] invalid wave duty id was calculated when determining channel 2 wave pattern: %d\n", waveDuty);
#endif
        exit(EXIT_FAILURE);
    }

    return wavePattern;
}

int Apu::DetermineChannel2FrequencyTimerValue() {
    uBYTE hiByte = memRef->rom[NR24] & 0b00000111;
    uBYTE loByte = memRef->rom[NR23];

    int frequencyQuotient = (hiByte << 8) | loByte;

    // These values were pulled from the gbdev pandocs
    // https://gbdev.io/pandocs/Sound_Controller.html#ff19---nr24---channel-2-frequency-hi-data-rw
    return (131072 / (2048 - frequencyQuotient));
}

uBYTE Apu::ComputeChannel2Amplitude() {
    uBYTE ch2Amplitude = 0;

    // First, decrement the frequency timer for the channel
    ch2FrequencyTimer--;

    // If this results in the timer reaching 0, then we
    // reset the timer's value with what is in NR24 (bits 2-0) | NR23 (11 bit value).
    // we also increment the wave pattern pointer when this happens.
    if (ch2FrequencyTimer <= 0) {
        ch2FrequencyTimer = DetermineChannel2FrequencyTimerValue();

        // The pointer value can only be within 0-7, and loops back once it
        // reaches 8.
        ch2WaveDutyPointer = (ch2WaveDutyPointer + 1) % 8;
    }

    // Finally, compute the channel's amplitude
    uBYTE wavePattern = DetermineChannel2WavePattern();
    if (wavePattern & (1 << ch2WaveDutyPointer)) {
        ch2Amplitude = 255;
    }
    else {
        ch2Amplitude = 0;
    }

    return ch2Amplitude;
}
