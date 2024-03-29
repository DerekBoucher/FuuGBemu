#include "Apu.hpp"

// Various sound functions
void lengthFunction(uBYTE& lengthTimer, bool& chDisabled);
void frequencySweepFunction(uBYTE& sweepTimer,
    int& chFrequency,
    int& chShadowFrequency,
    bool& chDisabled,
    bool sweepEnabled,
    int sweepPeriod,
    bool sweepDirection,
    uBYTE sweepShift);
void volumeEnvelopeFunction(int& volumeTimer, uBYTE& currentVolume, uBYTE volumePeriod, bool envelopeDirection);
int determineChannelFrequency(bool direction, uBYTE shiftAmount, int shadowFrequency);
uBYTE applyVolume(uBYTE rawVolume, uBYTE maxValue, uBYTE amplitude);
uBYTE determineSquareWavePattern(uBYTE nrx1);
int determineSquareWaveFrequencyTimerValue(uBYTE hiByte, uBYTE loByte, uBYTE multiplier);

Apu::Apu() {
    // Sampling specification for the pulse audio client
    //
    // Format:      Unsigned 8-bit PCM encoded sound data. This is the same format
    //              of the data that the DMG Gameboy sent to its DAC, and produces
    //              the iconic retro tones.
    //  Rate:       48000Hz sampling rate. This essentially means this emulator is sending the
    //              audio server 48000 PCM data points per second.
    //  Channels:   2. This is because the original DMG supported stereo sound with its two output speakers.
    pa_sample_spec samplingSpec = pa_sample_spec();
    samplingSpec.format = PA_SAMPLE_U8;
    samplingSpec.rate = AUDIO_SAMPLING_FREQUENCY_HZ;
    samplingSpec.channels = 2;

    // Initialize the pulse audio client
    int errorCode = 0;
    audioClient = pa_simple_new(NULL,
        "FuuGBEmuAPU",
        PA_STREAM_PLAYBACK,
        NULL,
        "FuuGBEmuAPU",
        &samplingSpec,
        NULL,
        NULL,
        &errorCode);

    // Error checking
    if (errorCode) {
        fprintf(stderr, "error initializing audio: %s\n", pa_strerror(errorCode));
        exit(EXIT_FAILURE);
    }

    // This timer controls when the APU samples the DMG's sound channels.
    // Since the DMG gameboy's CPU executes 4194304 cycles per second, and
    // that our sampling rate is 48000 samples per second, taking the division of
    // these two quantities will produce the amount of CPU cycles / sample that need
    // to have been executed before adding a sample to our audio buffer. 
    // (4194304 cycles/s / 48000 samples/s = ~87 cycles/sample)
    addToBufferTimer = CPU_FREQUENCY_HZ / AUDIO_SAMPLING_FREQUENCY_HZ;

    // Square wave with frequency sweep - Channel 1
    ch1Disabled = true;
    ch1VolumeTimer = 0;
    ch1WaveDutyPointer = 0;
    ch1ShadowFrequency = 0;
    ch1LengthTimer = 1;
    ch1FrequencyTimer = 0;
    ch1VolumeTimer = 0;

    // Square wave - Channel 2
    ch2VolumeTimer = 0;
    ch2LengthTimer = 1;
    ch2FrequencyTimer = 0;
    ch2WaveDutyPointer = 0;
    ch2CurrentVolume = 0;
    ch2Disabled = true;

    // Wave RAM - Channel 3
    ch3FrequencyTimer = 0;
    ch3WavePointer = 0;
    ch3Disabled = true;
    ch3LengthTimer = 1;
    ch3SamplePointer = 0;

    // Pointer in audio buffer for placement of the next audio samples.
    currentSampleBufferPosition = 0;

    // This timer value indicates the amount of CPU cycles that must have occured before
    // stepping the frame sequencer.
    frameSequencerTimer = CPU_FREQUENCY_HZ / FRAME_SEQUENCER_FREQUENCY_HZ;
    frameSequencerStep = 0;

    // Emulator specific channel toggles
    debuggerCh1Toggle = true;
    debuggerCh2Toggle = true;

    memset(audioBuffer, 0x00, AUDIO_BUFFER_SIZE);
}

void Apu::SetMemory(Memory* memRef) {
    this->memRef = memRef;
}

Apu::~Apu() {
    // Drain any remaining audio samples sent to the audio server.
    int errorCode = 0;
    pa_simple_drain(audioClient, &errorCode);
    if (errorCode) {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "error flushing audio buffer: %s\n", pa_strerror(errorCode));
#endif
        exit(EXIT_FAILURE);
    }

    pa_simple_free(audioClient);
}

void Apu::FlushBuffer() {
    int errorCode = 0;
    pa_simple_write(audioClient, audioBuffer, AUDIO_BUFFER_SIZE, &errorCode);
    if (errorCode) {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
#endif
        exit(EXIT_FAILURE);
    }
}

// Main sound routine for the APU.
void Apu::UpdateSound(int cycles) {
    UpdateFrameSequencer(cycles);

    // Compute the individual channels' amplitudes.
    // The original DMG has 4 distinct sounds that it can produce:
    //      1.  Square wave with a frequency sweep function
    //      2.  Square wave
    //      3.  Cartridge defined sound
    //      4.  Noise
    // Here we have to keep track of two copies of all channel's amplitudes;
    // one for the left output and another for the right output (stereo sound).
    uBYTE leftCh1Amplitude = ComputeChannel1Amplitude(cycles);
    uBYTE leftCh2Amplitude = ComputeChannel2Amplitude(cycles);
    uBYTE leftCh3Amplitude = ComputeChannel3Amplitude(cycles);
    uBYTE rightCh1Amplitude = leftCh1Amplitude;
    uBYTE rightCh2Amplitude = leftCh2Amplitude;
    uBYTE rightCh3Amplitude = leftCh3Amplitude;

    // NR50 - Enable Left (bit 7) (Unused) / Left speaker volume (bit 6-4) / Enable Right (bit 3) (Unused) / Right speaker volume bit (2-0)
    // https://gbdev.io/pandocs/Sound_Controller.html#ff24---nr50---channel-control--on-off--volume-rw
    uBYTE nr50 = memRef->rom[NR50];

    // NR51 - Sound panning for Channel 1,2,3 and 4.
    // https://gbdev.io/pandocs/Sound_Controller.html#ff25---nr51---selection-of-sound-output-terminal-rw
    uBYTE nr51 = memRef->rom[NR51];

    // NR52 - Master sound enable (bit 7) / Channel 1,2,3 and 4 Enable (bits 3-0, respectively)
    // https://gbdev.io/pandocs/Sound_Controller.html#ff26---nr52---sound-onoff
    uBYTE nr52 = memRef->rom[NR52];

    bool ch1LeftEnabled = nr51 & (1 << 4);
    bool ch2LeftEnabled = nr51 & (1 << 5);
    bool ch3LeftEnabled = nr51 & (1 << 6);
    // bool ch4LeftEnabled = nr51 & (1 << 7);

    bool ch1RightEnabled = nr51 & (1 << 0);
    bool ch2RightEnabled = nr51 & (1 << 1);
    bool ch3RightEnabled = nr51 & (1 << 2);
    // bool ch4RightEnabled = nr51 & (1 << 3);

    if (!ch1LeftEnabled)
        leftCh1Amplitude = 0;

    if (!ch1RightEnabled)
        rightCh1Amplitude = 0;

    if (!ch2LeftEnabled)
        leftCh2Amplitude = 0;

    if (!ch2RightEnabled)
        rightCh2Amplitude = 0;

    if (!ch3LeftEnabled)
        leftCh3Amplitude = 0;

    if (!ch3RightEnabled)
        rightCh3Amplitude = 0;

    // Mix the amplitudes of all channels
    uBYTE leftSample = (leftCh1Amplitude + leftCh2Amplitude + leftCh3Amplitude) / 3;
    uBYTE rightSample = (rightCh1Amplitude + rightCh2Amplitude + rightCh3Amplitude) / 3;

    // Apply left volume
    leftSample = applyVolume((nr50 & (0b01110000)) >> 4, 0x7, leftSample);

    // Apply Right volume
    rightSample = applyVolume((nr50 & (0b00000111)), 0x7, rightSample);

    // First, check if it is time to add a sample to the
    // audio buffer. We only need to send samples every
    // CPU_FREQUENCY_HZ / AUDIO_SAMPLING_FREQUENCY_HZ cycles.
    addToBufferTimer -= cycles;
    if (addToBufferTimer > 0)
        return;

    // If it is time to add new samples, refresh the timer value.
    // Here we add addToBufferTimer's cycles that might've resulted in a negative value, for better accuracy.
    addToBufferTimer = (CPU_FREQUENCY_HZ / AUDIO_SAMPLING_FREQUENCY_HZ) + addToBufferTimer;

    // Stuff the current samples in the buffer,
    // Since we are dealing with stereo sound, and that the audio client has been
    // configured with 2 channels, the audio samples must be interleaved in the following fashion:
    // LR,LR,LR,...
    audioBuffer[currentSampleBufferPosition++] = leftSample;
    audioBuffer[currentSampleBufferPosition++] = rightSample;

    // Play the sound if the buffer is full.
    // Reset the buffer pointer back to 0.
    // Optimization: only flush the buffer if at least one entry is non-zero.
    if (currentSampleBufferPosition >= AUDIO_BUFFER_SIZE) {
        currentSampleBufferPosition = 0;

        // If bit 7 of NR52 is reset, this means that the volume
        // is completely shut off. Only flush the buffer if the master
        // switch is ON.
        if ((nr52 & (1 << 7))) {
            FlushBuffer();
        }
    }
}

uBYTE Apu::ComputeChannel1Amplitude(int cycles) {
    uBYTE ch1Amplitude = 0;

    uBYTE nr10 = memRef->rom[NR10];
    uBYTE nr11 = memRef->rom[NR11];
    uBYTE nr12 = memRef->rom[NR12];
    uBYTE nr13 = memRef->rom[NR13];
    uBYTE nr14 = memRef->rom[NR14];

    bool lengthEnabled = nr14 & (1 << 6);
    bool volumeDirection = nr12 & (1 << 3);
    uBYTE lengthPeriod = nr11 & 0b00111111;
    uBYTE volumePeriod = nr12 & 0b00000111;
    uBYTE initialVolume = (nr12 & 0b11110000) >> 4;

    uBYTE sweepPeriod = (nr10 & (0b01110000)) >> 4;
    uBYTE sweepShift = nr10 & 0b00000111;
    bool sweepDirection = nr10 & 0b00001000;
    bool sweepEnabled = false;

    // Trigger event for channel 1 (when bit 7 of NR14 is set)
    // The following occur:
    //  - The channel is re-enabled
    //  - If the channel's length timer is 0, it is reloaded with value 64.
    //  - The channel's frequency timer is reloaded with nr14 | nr13
    //  - The volume timer for the volume envelope is refreshed with the volume period (bits 2-0 in NR12)
    //  - The current volume is refreshed with the initial volume (bits 7-4 in NR12)
    //  - The shadow frequency is refreshed with the current frequency
    //  - The frequency sweep timer is refreshed with the sweep period (bits 6-4 in NR10)
    //  - If the sweep  (or timer, in this case) was 0, the timer defaults to 8
    //  - If the sweep period OR the sweep shift amount are non-zero, then frequency sweeping is enabled
    //  - If the sweep shift is non-zero, recalculate the new frequency value to check if there's a resulting overflow
    //  - If there is an overflow, disable the channel
    if (memRef->TriggerEventCh1()) {
        ch1Disabled = false;

        if (ch1LengthTimer == 0)
            ch1LengthTimer = 64 - lengthPeriod;

        ch1FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr14, nr13, 4);
        ch1VolumeTimer = volumePeriod;
        ch1CurrentVolume = initialVolume;
        ch1ShadowFrequency = ch1FrequencyTimer;
        ch1SweepTimer = sweepPeriod;

        if (ch1SweepTimer == 0)
            ch1SweepTimer = 8;

        sweepEnabled = false;

        if (sweepPeriod > 0 || sweepShift > 0)
            sweepEnabled = true;

        if (sweepShift > 0) {
            ch1FrequencyTimer = determineChannelFrequency(sweepDirection, sweepShift, ch1ShadowFrequency);

            if (ch1FrequencyTimer > 2047) {
                ch1Disabled = true;
            }
        }
    }

    // If the frame sequencer clocks a sweep tick,
    // apply the frequency sweep function.
    if (sweepTick)
        frequencySweepFunction(ch1SweepTimer,
            ch1FrequencyTimer,
            ch1ShadowFrequency,
            ch1Disabled,
            sweepEnabled,
            sweepPeriod,
            sweepDirection,
            sweepShift);

    // substract the amount of cpu cycles that have occured from the
    // frequency timer.
    ch1FrequencyTimer -= cycles;

    // If this results in the frequency timer reaching 0, then we
    // reset the timer's value with what is in NR24 (bits 2-0) | NR23 (8 bit value).
    // we also increment the wave pattern pointer when this happens.
    if (ch1FrequencyTimer <= 0) {
        ch1FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr14, nr13, 4) + ch1FrequencyTimer;

        // The pointer value can only be within 0-7, and loops back once it
        // reaches 8.
        ch1WaveDutyPointer = (ch1WaveDutyPointer + 1) % 8;
    }

    // Fetch the raw wave amplitude for channel 1's wave pattern.
    // If the bit at the pointer is '1', then the amplitude is at its maximum value of 255.
    uBYTE wavePattern = determineSquareWavePattern(nr11);
    if (wavePattern & (1 << ch1WaveDutyPointer))
        ch1Amplitude = 255;

    // If the frame sequencer ticked a volume envelope, then apply the volume envelope on the volume.
    if (volumeEnvelopeTick)
        volumeEnvelopeFunction(ch1VolumeTimer, ch1CurrentVolume, volumePeriod, volumeDirection);

    // Apply volume to amplitude.
    ch1Amplitude = applyVolume(ch1CurrentVolume, 0xF, ch1Amplitude);


    // If the CPU wrote a value to NR11, then this indicates that we
    // need to restore the timer's value with 64 - period.
    if (memRef->RequiresCh1LengthReload())
        ch1LengthTimer = 64 - lengthPeriod;

    // If the frame sequencer clocked a length tick, apply the length function to the channel.
    if (lengthControlTick && lengthEnabled)
        lengthFunction(ch1LengthTimer, ch1Disabled);

    // If channel is disabled, amplitude drops to 0.
    if (ch1Disabled)
        ch1Amplitude = 0;

    // If the user turned the channel off through the debugger, the amplitude drops to 0.
    if (!debuggerCh1Toggle)
        ch1Amplitude = 0;

    return ch1Amplitude;
}

uBYTE Apu::ComputeChannel2Amplitude(int cycles) {
    uBYTE ch2Amplitude = 0;

    uBYTE nr21 = memRef->rom[NR21];
    uBYTE nr22 = memRef->rom[NR22];
    uBYTE nr23 = memRef->rom[NR23];
    uBYTE nr24 = memRef->rom[NR24];

    bool lengthEnabled = nr24 & (1 << 6);
    bool volumeDirection = nr22 & (1 << 3);
    uBYTE lengthPeriod = nr21 & 0b00111111;
    uBYTE volumePeriod = nr22 & 0b00000111;
    uBYTE initialVolume = (nr22 & 0b11110000) >> 4;

    // Trigger event for channel 2 (when bit 7 of NR24 is set)
    // The following occur:
    //  - The channel is re-enabled
    //  - If the channel's length timer is 0, it is reloaded with value 64.
    //  - The channel's frequency timer is reloaded with nr24 | nr23
    //  - The volume timer for the volume envelope is refreshed with the volume period (bits 2-0 in NR22)
    //  - The current volume is refreshed with the initial volume (bits 7-4 in NR22)
    if (memRef->TriggerEventCh2()) {
        ch2Disabled = false;

        if (ch2LengthTimer == 0)
            ch2LengthTimer = 64 - lengthPeriod;

        ch2FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr24, nr23, 4);
        ch2VolumeTimer = volumePeriod;
        ch2CurrentVolume = initialVolume;
    }

    // First, substract the amount of cpu cycles that have occured from the
    // frequency timer.
    ch2FrequencyTimer -= cycles;

    // If this results in the frequency timer reaching 0, then we
    // reset the timer's value with what is in NR24 (bits 2-0) | NR23 (8 bit value).
    // we also increment the wave pattern pointer when this happens.
    if (ch2FrequencyTimer <= 0) {
        ch2FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr24, nr23, 4) + ch2FrequencyTimer;

        // The pointer value can only be within 0-7, and loops back once it
        // reaches 8.
        ch2WaveDutyPointer = (ch2WaveDutyPointer + 1) % 8;
    }

    // Fetch the raw wave amplitude for channel 2's wave pattern.
    // If the bit at the pointer is '1', then the amplitude is at its maximum value of 255.
    uBYTE wavePattern = determineSquareWavePattern(nr21);
    if (wavePattern & (1 << ch2WaveDutyPointer))
        ch2Amplitude = 255;

    // If the frame sequencer ticked a volume envelope, then apply the volume envelope on the volume.
    if (volumeEnvelopeTick)
        volumeEnvelopeFunction(ch2VolumeTimer, ch2CurrentVolume, volumePeriod, volumeDirection);

    // Apply volume to amplitude.
    ch2Amplitude = applyVolume(ch2CurrentVolume, 0xF, ch2Amplitude);

    // If the CPU wrote a value to NR21, then this indicates that we
    // need to restore the timer's value with 64 - period.
    if (memRef->RequiresCh2LengthReload())
        ch2LengthTimer = 64 - lengthPeriod;

    // If the frame sequencer clocked a length tick, apply the length function to the channel.
    if (lengthControlTick && lengthEnabled)
        lengthFunction(ch2LengthTimer, ch2Disabled);

    // If channel is disabled, amplitude drops to 0.
    if (ch2Disabled)
        ch2Amplitude = 0;

    // If the user turned the channel off through the debugger, the amplitude drops to 0.
    if (!debuggerCh2Toggle)
        ch2Amplitude = 0;

    return ch2Amplitude;
}

uBYTE Apu::ComputeChannel3Amplitude(int cycles) {
    uBYTE ch3Amplitude = 0;

    uBYTE nr30 = memRef->rom[NR30];
    uBYTE nr31 = memRef->rom[NR31];
    uBYTE nr32 = memRef->rom[NR32];
    uBYTE nr33 = memRef->rom[NR33];
    uBYTE nr34 = memRef->rom[NR34];

    bool channelEnabled = nr30 & (1 << 7);
    bool lengthEnabled = nr34 & (1 << 6);
    uBYTE lengthPeriod = nr31;
    uBYTE volumeShift = (nr32 & 0b01100000) >> 5;


    // Trigger event for channel 3 (when bit 7 of NR34 is set)
    // The following occur:
    //  - The channel is re-enabled
    //  - If the channel's length timer is 0, it is reloaded with value 256.
    //  - The channel's frequency timer is reloaded with nr34 | nr33
    if (memRef->TriggerEventCh3()) {
        ch3Disabled = false;

        if (ch3LengthTimer == 0)
            ch3LengthTimer = 255 - lengthPeriod;

        ch3FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr34, nr33, 2);
    }

    // First, substract the amount of cpu cycles that have occured from the
    // frequency timer.
    ch3FrequencyTimer -= cycles;

    // If this results in the frequency timer reaching 0, then we
    // reset the timer's value with what is in NR24 (bits 2-0) | NR23 (8 bit value).
    // we also increment the wave pattern pointer when this happens.
    if (ch3FrequencyTimer <= 0) {
        ch3FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr34, nr33, 2) + ch3FrequencyTimer;

        // The pointer value can only be within 0-7, and loops back once it
        // reaches 8.
        ch3WavePointer = ch3WavePointer + 1;

        // If the wave pointer equals 255, this means we have played an entire sample from the hi bits.
        // When this happens, we now play the next sample in the wave table which begins at FF30.
        // There are 16 samples in the wave table, which both contain 2 sub samples of 4-bit each.
        if (ch3WavePointer > 7) {
            ch3WavePointer = 0;
            ch3SamplePointer = (ch3SamplePointer + 1) % 16;
        }
    }

    uBYTE wavePattern = memRef->rom[WAVE_RAM + ch3SamplePointer];
    uBYTE upperSample = (wavePattern & 0b11110000) >> 4;
    uBYTE lowerSample = (wavePattern & 0b00001111);

    uBYTE shiftAmount;
    switch (volumeShift) {
    case 0: shiftAmount = 4; break;
    case 1: shiftAmount = 0; break;
    case 2: shiftAmount = 1; break;
    case 3: shiftAmount = 2; break;
    default: shiftAmount = 4; break;
    }

    if (upperSample > 0 || lowerSample > 0)
        int two = 2;

    if (ch3WavePointer < 4)
        ch3Amplitude = (upperSample >> shiftAmount);
    else
        ch3Amplitude = (lowerSample >> shiftAmount);


    // If the CPU wrote a value to NR21, then this indicates that we
    // need to restore the timer's value with 64 - period.
    if (memRef->RequiresCh3LengthReload())
        ch3LengthTimer = 255 - lengthPeriod;

    // If the frame sequencer clocked a length tick, apply the length function to the channel.
    if (lengthControlTick && lengthEnabled)
        lengthFunction(ch3LengthTimer, ch3Disabled);

    // If channel is disabled, amplitude drops to 0.
    if (ch3Disabled || !channelEnabled)
        ch3Amplitude = 0;

    return ch3Amplitude;
}

void Apu::UpdateFrameSequencer(int cycles) {
    frameSequencerTimer -= cycles;

    // Clear the state of the ticks.
    lengthControlTick = false;
    sweepTick = false;
    volumeEnvelopeTick = false;

    // If the timer hasn't reached 0 yet, it isn't timer for
    // an FS clock tick of any kind.
    if (frameSequencerTimer > 0)
        return;

    // If we reach this point, it's time to generate a new FS clock tick.
    frameSequencerStep = (frameSequencerStep + 1) % 8;
    frameSequencerTimer = (CPU_FREQUENCY_HZ / FRAME_SEQUENCER_FREQUENCY_HZ) + frameSequencerTimer;

    // Depending on the step value of the frame sequencer, different clock ticks
    // get generated. See the Frame sequencer section in the below article.
    // https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Channels
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

int determineChannelFrequency(bool direction, uBYTE shiftAmount, int shadowFrequency) {
    int newFrequency = shadowFrequency >> shiftAmount;

    if (direction == INCREASING) {
        newFrequency = shadowFrequency + newFrequency;
    }

    if (direction == DECREASING) {
        newFrequency = shadowFrequency - newFrequency;
    }

    return newFrequency;
}

void frequencySweepFunction(uBYTE& sweepTimer,
    int& chFrequency,
    int& chShadowFrequency,
    bool& chDisabled,
    bool sweepEnabled,
    int sweepPeriod,
    bool sweepDirection,
    uBYTE sweepShift) {
    if (sweepTimer > 0) {
        sweepTimer--;
    }

    // Only proceed with the frequency sweep
    // if and only if the sweep timer became 0
    if (sweepTimer != 0) {
        return;
    }

    // Reload the sweep timer with the period.
    // If NR10 contained a 0 period, the timer 
    // defaults to 8.
    sweepTimer = sweepPeriod;
    if (sweepTimer == 0) {
        sweepTimer = 8;
    }

    if (sweepEnabled && sweepPeriod > 0) {
        int newFrequency = determineChannelFrequency(sweepDirection, sweepShift, chShadowFrequency);

        if (newFrequency < 2048 && sweepShift > 0) {
            chFrequency = newFrequency;
            chShadowFrequency = newFrequency;

            // Recalculate the frequency, simply to detect if
            // it overflows with these new shadow frequencies.
            newFrequency = determineChannelFrequency(sweepDirection, sweepShift, chShadowFrequency);
        }

        if (newFrequency > 2047) {
            chDisabled = true;
        }
    }
}

void volumeEnvelopeFunction(int& volumeTimer, uBYTE& currentVolume, uBYTE volumePeriod, bool envelopeDirection) {
    // If the period is 0,
    // nothing to do.
    if (volumePeriod == 0) {
        return;
    }

    if (volumeTimer > 0) {
        volumeTimer--;
    }

    // Only proceed with the volume envelope
    // if the decrement resulted in the period timer becoming 0
    if (volumeTimer != 0) {
        return;
    }

    // Reload the volume timer
    volumeTimer = volumePeriod;
    if (volumeTimer == 0) {
        volumeTimer = 8;
    }

    // Update the current volume
    if (envelopeDirection == INCREASING && currentVolume < 0xF) {
        currentVolume++;
        return;
    }

    if (envelopeDirection == DECREASING && currentVolume > 0x0) {
        currentVolume--;
        return;
    }
}

uBYTE applyVolume(uBYTE rawVolume, uBYTE maxValue, uBYTE amplitude) {
    uBYTE volumeModifier = 255 - ((255 / maxValue) * rawVolume);

    // If the raw volume value equates the max potential value,
    // This means that we should not attempt to substract anything
    // from the amplitude.
    if (rawVolume == maxValue)
        volumeModifier = 0;

    // Conversely, if the rawVolume is 0, this indicates
    // that the intention is to completely mute the amplitude,
    // therefore we must substract the maximum 8-bit value of 255.
    if (rawVolume == 0)
        volumeModifier = 255;

    // If the volume modifier computed exceeds the amplitude given
    // simply set the amplitude to 0. Trying to substract the two amounts
    // would cause the amplitude to underflow and produce an incorrect value.
    if (volumeModifier >= amplitude) {
        amplitude = 0;
    }

    // Else, simply take the difference.
    else {
        amplitude -= volumeModifier;
    }

    return amplitude;
}

uBYTE determineSquareWavePattern(uBYTE nrx1) {
    // Wave duty identifier is found in bit 7-6 of NRx1
    // Only applies to channels 1 and 2
    uBYTE waveDuty = ((nrx1 & (0b11000000)) >> 6);
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

void lengthFunction(uBYTE& lengthTimer, bool& chDisabled) {
    if (lengthTimer > 0) {
        lengthTimer--;
    }

    if (lengthTimer == 0) {
        chDisabled = true;
    }
}

int determineSquareWaveFrequencyTimerValue(uBYTE hiByte, uBYTE loByte, uBYTE multiplier) {
    int frequencyQuotient = ((hiByte & 0x7) << 8) | loByte;

    return (2048 - frequencyQuotient) * multiplier;
}
