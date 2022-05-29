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
    frameSequencerTimer = FRAME_SEQUENCER_CYCLES;
    frameSequencerStep = 0;
    ch1VolumeTimer = 0;
    ch1ShadowFrequency = 0;
    ch1Frequency = 0;
    ch1LengthTimer = 64;
    ch1Disabled = true;
    ch1FrequencyTimer = 0;
    ch2Disabled = true;
    ch1VolumeTimer = 0;
    ch2CurrentVolume = 0;
}

Apu::~Apu() {
    pa_simple_free(audioClient);
}

void Apu::FlushBuffer() {
    int errorCode = 0;

    pa_simple_write(audioClient, buffer, BUFFER, &errorCode);
    if (errorCode) {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
#endif
        exit(EXIT_FAILURE);
    }

    pa_simple_drain(audioClient, &errorCode);
    if (errorCode) {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "error flushing audio buffer: %s\n", pa_strerror(errorCode));
#endif
        exit(EXIT_FAILURE);
    }
}

void Apu::UpdateSoundRegisters(int cycles) {
    UpdateFrameSequencer(cycles);

    // First, check if it is time to add a sample to the
    // audio buffer. We only need to send samples every
    // CPU_FREQUENCY / APU_SAMPLE_RATE cycles (which equates to 87 cycles, in this case)
    addToBufferTimer -= cycles;
    if (addToBufferTimer > 0) {
        return;
    }

    // If it is time to add new samples, refresh the timer value.
    // Here we add addToBufferTimer's cycles that might've resulted in a negative value
    addToBufferTimer = (CPU_FREQUENCY / APU_SAMPLE_RATE) + addToBufferTimer;

    // Channel 2
    uBYTE ch2Amplitude = ComputeChannel2Amplitude();

    // Mix the amplitudes of all channels
    uBYTE sample = 128 + ((ch2Amplitude) / 4);

    uBYTE nr50 = memRef->rom[NR50];
    uBYTE nr51 = memRef->rom[NR51];
    uBYTE nr52 = memRef->rom[NR52];

    // If we are currently working with the right
    // stereo sample, modulo 2 returns 1
    if (currentSampleBufferPosition % 2) {
        // Verify if NR51 register disables the right output for channel 2
        if ((nr51 & (1 << 5)) == 0) {
            sample = 128;
        }
    }
    else { // else we are dealing with the left stereo sample

        // Verify if NR51 register disables the left output for channel 2
        if ((nr51 & (1 << 1)) == 0) {
            sample = 128;
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

    // Normalize volume modifier WRT 128 silence value
    switch (volume) {
    case 0: volume = 128; break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
        volume = 128 - ((128 / 7) * volume);
        break;
    case 7: volume = 0; break;
    default:
#ifdef FUUGB_DEBUG
        fprintf(stderr, "[APU] invalid global volume modifier in NR50 computed.");
#endif
        exit(EXIT_FAILURE);
    }

    sample = sample - volume;

    // TODO: Determine what to do with NR52 bits 0-3

    // Finally, stuff the sample in the buffer,
    // and play the sound if the buffer 0b11111100is full
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
    uBYTE waveDuty = ((memRef->rom[NR21] & (0b11000000)) >> 6);
    uBYTE wavePattern = 0;

    switch (waveDuty) {
    case 0: wavePattern = DUTY1; break;
    case 1: wavePattern = DUTY2; break;
    case 2: wavePattern = DUTY3; break;
    case 3: wavePattern = DUTY4; break;
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

uBYTE Apu::ComputeChannel1Ampltiude() {
    uBYTE ch1Amplitude = 0;

    uBYTE nr10 = memRef->rom[NR10];
    uBYTE nr11 = memRef->rom[NR11];
    uBYTE nr12 = memRef->rom[NR12];
    uBYTE nr13 = memRef->rom[NR13];
    uBYTE nr14 = memRef->rom[NR14];

    uBYTE volumePeriod = nr12 & (0b00000111);
    uBYTE initialVolume = (nr12 & (0b11110000)) >> 4;
    uBYTE sweepPeriod = (nr10 & (0b01110000)) >> 4;
    uBYTE sweepShift = nr10 & 0b00000111;
    uBYTE wavePatternDuty = (nr11 & 0b11000000) >> 6;
    uBYTE lengthPeriod = nr11 & 0b00111111;

    bool volumeDirection = nr12 & 0b00001000;
    bool sweepDirection = nr10 & 0b00001000;
    bool lengthEnabled = nr14 & 0b01000000;
    bool sweepEnabled = false;

    // Trigger event for channel 1 (when bit 7 of NR14 is set)
    // The following occur:
    //  - The channel is re-enabled
    //  - The volume timer for the volume envelope is refreshed with the volume period (bits 2-0 in NR12)
    //  - The current volume is refreshed with the initial volume (bits 7-4 in NR12)
    //  - The shadow frequency is refreshed with the current frequency
    //  - The frequency sweep timer is refreshed with the sweep period (bits 6-4 in NR10)
    //  - If the sweep period was 0, the timer defaults to 8
    //  - If the sweep period OR the sweep shift amount are non-zero, then frequency sweeping is enabled
    //  - If the sweep shift is non-zero, recalculate the new frequency value to check if there's a resulting overflow
    //  - If there is an overflow, disable the channel
    //  - The length timer is reloaded with 64 - lengthPeriod, if it was 0
    if (nr14 & (1 << 7)) {
        ch1Disabled = false;
        ch1VolumeTimer = volumePeriod;
        ch1CurrentVolume = initialVolume;
        ch1ShadowFrequency = ch1Frequency;
        ch1SweepTimer = sweepPeriod;
        if (ch1SweepTimer == 0) {
            ch1SweepTimer = 8;
        }

        if (sweepPeriod > 0 || sweepShift > 0) {
            sweepEnabled = true;
        }

        if (sweepShift > 0) {
            int newFrequency = DetermineChannel1Frequency(sweepDirection, sweepShift);

            if (newFrequency > 2047) {
                ch1Disabled = true;
            }
        }

        if (ch1LengthTimer == 0) {
            ch1LengthTimer = 64 - lengthPeriod;
        }
    }

    if (volumeEnvelopeTick) {

        // If the period in NR12 is 0,
        // nothing to do.
        if (volumePeriod == 0) {
            goto skip_volume_env_1;
        }

        ch1VolumeTimer--;

        // Only proceed with the volume envelope
        // if the decrement resulted in the period timer becoming 0
        if (ch1VolumeTimer != 0) {
            goto skip_volume_env_1;
        }

        // Reload the volume timer
        ch1VolumeTimer = volumePeriod;

        // Update the current volume
        if (volumeDirection == INCREASING && ch1CurrentVolume < 0xF) {
            ch1CurrentVolume++;
        }

        if (volumeDirection == DECREASING && ch1CurrentVolume > 0x0) {
            ch1CurrentVolume--;
        }
    }

skip_volume_env_1:

    if (sweepTick) {

        if (ch1SweepTimer > 0) {
            ch1SweepTimer--;
        }

        // Only proceed with the frequency sweep
        // if and only if the sweep timer became 0
        if (ch1SweepTimer != 0) {
            goto skip_sweep_env_1;
        }

        // Reload the sweep timer with the period.
        // If NR10 contained a 0 period, the timer 
        // defaults to 8.
        ch1SweepTimer = sweepPeriod;
        if (ch1SweepTimer == 0) {
            ch1SweepTimer = 8;
        }

        if (sweepEnabled && sweepPeriod > 0) {
            int newFrequency = DetermineChannel1Frequency(sweepDirection, sweepShift);

            if (newFrequency < 2048 && sweepShift > 0) {
                ch1Frequency = newFrequency;
                ch1ShadowFrequency = newFrequency;

                // Recalculate the frequency, simply to detect if
                // it overflows with these new shadow frequencies.
                newFrequency = DetermineChannel1Frequency(sweepDirection, sweepShift);
            }

            if (newFrequency > 2047) {
                ch1Disabled = true;
            }
        }
    }

skip_sweep_env_1:

    if (lengthControlTick && lengthEnabled) {
        ch1LengthTimer--;

        if (ch1LengthTimer == 0) {
            ch1Disabled = true;
        }
    }

    return ch1Amplitude;
}

uBYTE Apu::ComputeChannel2Amplitude() {
    uBYTE ch2Amplitude = 0;

    uBYTE nr21 = memRef->rom[NR21];
    uBYTE nr22 = memRef->rom[NR22];
    uBYTE nr24 = memRef->rom[NR24];

    bool lengthEnabled = nr24 & 0b01000000;
    bool volumeDirection = nr22 & 0b00001000;
    uBYTE lengthData = nr21 & 0b00111111;
    uBYTE volumePeriod = (nr22 & 0b00000111);
    uBYTE initialVolume = (nr22 & 0b11110000) >> 4;

    if (nr24 & (1 << 7)) {
        memRef->rom[NR24] = nr24 & ~(1 << 7);

        ch2Disabled = false;
        ch2LengthTimer = 64 - lengthData;
        ch2VolumeTimer = volumePeriod;
        ch2CurrentVolume = initialVolume;
    }

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
        ch2Amplitude = 127;
    }
    else {
        ch2Amplitude = 0;
    }

    if (volumeEnvelopeTick) {

        // If the period in NR22 is 0,
        // nothing to do.
        if (volumePeriod == 0) {
            goto skip_volume_env_2;
        }

        if (ch2VolumeTimer > 0) {
            ch2VolumeTimer--;
        }

        // Only proceed with the volume envelope
        // if the decrement resulted in the period timer becoming 0
        if (ch2VolumeTimer != 0) {
            goto skip_volume_env_2;
        }

        // Reload the volume timer
        ch2VolumeTimer = volumePeriod;

        // Update the current volume
        if (volumeDirection == INCREASING && ch2CurrentVolume < 0xF) {
            ch2CurrentVolume++;
        }

        if (volumeDirection == DECREASING && ch2CurrentVolume > 0x0) {
            ch2CurrentVolume--;
        }
    }

skip_volume_env_2:

    if (lengthControlTick && lengthEnabled) {
        if (ch2LengthTimer > 0) {
            ch2LengthTimer--;
        }

        if (ch2LengthTimer == 0) {
            ch2Disabled = true;
        }
    }

    uBYTE volumeModifier;
    if (ch2CurrentVolume == 0) {
        volumeModifier = 127;
    }
    else if (ch2CurrentVolume == 0xF) {
        volumeModifier = 0;
    }
    else if (ch2CurrentVolume > 0 && ch2CurrentVolume < 0xF) {
        volumeModifier = 127 - ((127 / 0xF) * ch2CurrentVolume);
    }
    else {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "[APU] invalid current volume modifier computed for channel 2.");
#endif
        exit(EXIT_FAILURE);
    }

    ch2Amplitude -= volumeModifier;

    if (ch2Disabled) {
        ch2Amplitude = 0;
    }

    return ch2Amplitude;
}

void Apu::UpdateFrameSequencer(int cycles) {
    frameSequencerTimer -= cycles;
    if (frameSequencerTimer <= 0) {
        frameSequencerStep = (frameSequencerStep + 1) % 8;
        frameSequencerTimer = FRAME_SEQUENCER_CYCLES + frameSequencerTimer;
    }

    // Clear the state of the ticks
    lengthControlTick = false;
    sweepTick = false;
    volumeEnvelopeTick = false;

    switch (frameSequencerStep) {
    case 1:
    case 3:
    case 5:
        break;
    case 0:
    case 4:
        lengthControlTick = true;
        break;
    case 2:
        lengthControlTick = true;
        sweepTick = true;
        break;
    case 6:
        lengthControlTick = true;
        sweepTick = true;
        break;
    case 7:
        volumeEnvelopeTick = true;
        break;
    default:
#ifdef FUUGB_DEBUG
        fprintf(stderr, "[APU] frame sequencer step was not within the 0-7 range");
#endif
        exit(EXIT_FAILURE);
    }
}

int Apu::DetermineChannel1Frequency(bool direction, uBYTE shiftAmount) {
    int newFrequency = ch1ShadowFrequency >> shiftAmount;

    if (direction == INCREASING) {
        newFrequency = ch1ShadowFrequency + newFrequency;
    }

    if (direction == DECREASING) {
        newFrequency = ch1ShadowFrequency - newFrequency;
    }

    return newFrequency;
}

uBYTE Apu::DetermineChannel1WavePattern() {
    // Wave duty identifier is found in bit 7-6 of NR11
    uBYTE waveDuty = ((memRef->rom[NR11] & (0b11000000)) >> 6);
    uBYTE wavePattern = 0;

    switch (waveDuty) {
    case 0: wavePattern = DUTY1; break;
    case 1: wavePattern = DUTY2; break;
    case 2: wavePattern = DUTY3; break;
    case 3: wavePattern = DUTY4; break;
    default:
#ifdef FUUGB_DEBUG
        fprintf(stderr, "[APU] invalid wave duty id was calculated when determining channel 1 wave pattern: %d\n", waveDuty);
#endif
        exit(EXIT_FAILURE);
    }

    return wavePattern;
}
