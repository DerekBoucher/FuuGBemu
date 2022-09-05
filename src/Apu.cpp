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
uBYTE determineChannelWavePattern(uBYTE nrx1);
int determineSquareWaveFrequencyTimerValue(uBYTE hiByte, uBYTE loByte);

Apu::Apu() {}

Apu::Apu(Memory* memRef) {
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

    this->memRef = memRef;

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
    ch1Frequency = 0;
    ch1LengthTimer = 64;
    ch1FrequencyTimer = 0;
    ch1VolumeTimer = 0;

    // Square wave - Channel 2
    ch2VolumeTimer = 0;
    ch2FrequencyTimer = 0;
    ch2WaveDutyPointer = 0;
    ch2CurrentVolume = 0;
    ch2Disabled = true;

    // Pointer in audio buffer for placement of the next audio samples.
    currentSampleBufferPosition = 0;

    // This timer value indicates the amount of CPU cycles that must have occured before
    // stepping the frame sequencer.
    frameSequencerTimer = CPU_FREQUENCY_HZ / FRAME_SEQUENCER_FREQUENCY_HZ;
    frameSequencerStep = 0;

    // Start the audio buffer flusher thread.
    flusherRunning = true;
    apuFlusher = std::unique_ptr<std::thread>(new std::thread(&Apu::FlusherRoutine, this));
}

Apu::~Apu() {
    // Stop the audio buffer flusher thread and wait for it.
    flusherRunning = false;
    loopCv.notify_one();
    if (apuFlusher->joinable()) {
        apuFlusher->join();
    }

    // Release the audio client resources.
    pa_simple_free(audioClient);
}

void Apu::FlusherRoutine() {

    // Pretty straight forward routine:
    //  1. Wait on a condition variable until the main gameboy thread signals it.
    //  2. If a signal comes in, flush the audio buffer.
    //  3. Repeat. (until the flusherRunning boolean is false)
    while (flusherRunning) {
        std::unique_lock<std::mutex> loopLock(loopMtx);
        loopCv.wait(loopLock);
        FlushBuffer();
        loopLock.unlock();
    }

    // Drain any remaining audio samples sent to the audio server.
    int errorCode = 0;
    pa_simple_drain(audioClient, &errorCode);
    if (errorCode) {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "error flushing audio buffer: %s\n", pa_strerror(errorCode));
#endif
        exit(EXIT_FAILURE);
    }
}

void Apu::NotifyFlusher() {
    loopCv.notify_one();
}

void Apu::FlushBuffer() {
    uBYTE bufferCopy[AUDIO_BUFFER_SIZE];

    // Make a copy of the audio buffer such as to get
    // a clean snapshot of the samples before the main thread starts
    // altering the buffer again.
    bufferLock.lock();
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++)
        bufferCopy[i] = audioBuffer[i];
    bufferLock.unlock();

    // Play the audio buffer.
    int errorCode = 0;
    pa_simple_write(audioClient, bufferCopy, AUDIO_BUFFER_SIZE, &errorCode);
    if (errorCode) {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "error playing back audio: %s\n", pa_strerror(errorCode));
#endif
        exit(EXIT_FAILURE);
    }
}

// Main sound routine for the APU.
void Apu::UpdateSoundRegisters(int cycles) {
    UpdateFrameSequencer(cycles);

    // Compute the individual channels' amplitudes.
    // The original DMG has 4 distinct sounds that it can produce:
    //      1.  Square wave with a frequency sweep function
    //      2.  Square wave
    //      3.  Noise
    //      4.  Cartridge defined sound
    // Here we have to keep track of two copies of all channel's amplitudes;
    // one for the left output and another for the right output (stereo sound).
    uBYTE leftCh2Amplitude = ComputeChannel2Amplitude(cycles);
    uBYTE rightCh2Amplitude = leftCh2Amplitude;

    // NR50 - Enable Left (bit 7) (Unused) / Left speaker volume (bit 6-4) / Enable Right (bit 3) (Unused) / Right speaker volume bit (2-0)
    // https://gbdev.io/pandocs/Sound_Controller.html#ff24---nr50---channel-control--on-off--volume-rw
    uBYTE nr50 = memRef->rom[NR50];

    // NR51 - Sound panning for Channel 1,2,3 and 4.
    // https://gbdev.io/pandocs/Sound_Controller.html#ff25---nr51---selection-of-sound-output-terminal-rw
    uBYTE nr51 = memRef->rom[NR51];

    // NR52 - Master sound enable (bit 7) / Channel 1,2,3 and 4 Enable (bits 3-0, respectively)
    // https://gbdev.io/pandocs/Sound_Controller.html#ff26---nr52---sound-onoff
    uBYTE nr52 = memRef->rom[NR52];

    bool ch2LeftEnabled = nr51 & (1 << 5);
    bool ch2RightEnabled = nr51 & (1 << 2);

    if (!ch2LeftEnabled)
        leftCh2Amplitude = 0;

    if (!ch2RightEnabled)
        rightCh2Amplitude = 0;

    // Mix the amplitudes of all channels
    uBYTE leftSample = leftCh2Amplitude;
    uBYTE rightSample = rightCh2Amplitude;

    // Apply left volume
    leftSample = applyVolume((nr50 & (0b01110000)) >> 4, 0x7, leftSample);

    // Apply Right volume
    rightSample = applyVolume((nr50 & (0b00000111)), 0x7, rightSample);

    // First, check if it is time to add a sample to the
    // audio buffer. We only need to send samples every
    // CPU_FREQUENCY_HZ / AUDIO_SAMPLING_FREQUENCY_HZ cycles.
    addToBufferTimer -= cycles;
    if (addToBufferTimer > 0) {
        return;
    }

    // If it is time to add new samples, refresh the timer value.
    // Here we add addToBufferTimer's cycles that might've resulted in a negative value, for better accuracy.
    addToBufferTimer = (CPU_FREQUENCY_HZ / AUDIO_SAMPLING_FREQUENCY_HZ) + addToBufferTimer;

    // Stuff the current samples in the buffer,
    // Since we are dealing with stereo sound, and that the audio client has been
    // configured with 2 channels, the audio samples must be interleaved in the following fashion:
    // LR,LR,LR,...
    bufferLock.lock();
    audioBuffer[currentSampleBufferPosition++] = leftSample;
    audioBuffer[currentSampleBufferPosition++] = rightSample;
    bufferLock.unlock();

    // Play the sound if the buffer is full.
    // Reset the buffer pointer back to 0.
    if (currentSampleBufferPosition >= AUDIO_BUFFER_SIZE) {
        currentSampleBufferPosition = 0;

        // If bit 7 of NR52 is reset, this means that the volume
        // is completely shut off. Only flush the buffer if the master
        // switch is ON.
        if ((nr52 & (1 << 7))) {
            NotifyFlusher();
        }
    }
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
    //  - The length timer is reloaded with 64, if it was 0
    if (nr14 & (1 << 7)) {
        memRef->rom[NR14] = nr14 & ~(1 << 7);

        ch1Disabled = false;

        if (ch1LengthTimer == 0)
            ch1LengthTimer = 64;

        ch1FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr14, nr13);
        ch1VolumeTimer = volumePeriod;
        ch1CurrentVolume = initialVolume;
        ch1ShadowFrequency = ch1Frequency;
        ch1SweepTimer = sweepPeriod;

        if (ch1SweepTimer == 0)
            ch1SweepTimer = 8;

        if (sweepPeriod > 0 || sweepShift > 0)
            sweepEnabled = true;
        else
            sweepEnabled = false;

        if (sweepShift > 0) {
            int newFrequency = determineChannelFrequency(sweepDirection, sweepShift, ch1ShadowFrequency);

            if (newFrequency > 2047) {
                ch1Disabled = true;
            }
        }
    }

    // First, decrement the frequency timer for the channel
    ch1FrequencyTimer--;

    // Fetch the raw wave pattern amplitude for channel 1
    uBYTE wavePattern = determineChannelWavePattern(nr11);
    if (wavePattern & (1 << ch1WaveDutyPointer)) {
        ch1Amplitude = 127;
    }
    else {
        ch1Amplitude = 0;
    }

    if (volumeEnvelopeTick) {
        volumeEnvelopeFunction(ch1VolumeTimer, ch1CurrentVolume, volumePeriod, volumeDirection);
    }

    if (sweepTick) {
        frequencySweepFunction(ch1SweepTimer,
            ch1Frequency,
            ch1ShadowFrequency,
            ch1Disabled,
            sweepEnabled,
            sweepPeriod,
            sweepDirection,
            sweepShift);
    }

    if (memRef->RequiresCh2LengthReload()) {
        ch2LengthTimer = 64 - lengthPeriod;
    }

    if (lengthControlTick && lengthEnabled) {
        lengthFunction(ch1LengthTimer, ch1Disabled);
    }

    return ch1Amplitude;
}

uBYTE Apu::ComputeChannel2Amplitude(int cycles) {
    uBYTE ch2Amplitude = 0;

    // NR21 - Channel 2 Sound length / Wave pattern duty
    // https://gbdev.io/pandocs/Sound_Controller.html#ff16---nr21---channel-2-sound-lengthwave-pattern-duty-rw
    uBYTE nr21 = memRef->rom[NR21];

    // NR22 - Channel 2 Volume envelope
    // https://gbdev.io/pandocs/Sound_Controller.html#ff17---nr22---channel-2-volume-envelope-rw
    uBYTE nr22 = memRef->rom[NR22];

    // NR23 - Channel 2 Frequency (Least significant byte)
    // https://gbdev.io/pandocs/Sound_Controller.html#ff18---nr23---channel-2-frequency-lo-data-w
    uBYTE nr23 = memRef->rom[NR23];

    // NR24 - Channel 2 Frequency (highest significant byte) + Channel enable
    // https://gbdev.io/pandocs/Sound_Controller.html#ff19---nr24---channel-2-frequency-hi-data-rw
    uBYTE nr24 = memRef->rom[NR24];

    // lengthEnabled determines if the channel should apply the length function
    // to the output amplitude
    bool lengthEnabled = nr24 & (1 << 6);

    // volumeDirection dictates which way the volume envelope function should alter the volume
    // of the output amplitude. True = increment volume, False = decrement volume.
    bool volumeDirection = nr22 & (1 << 3);

    // lengthPeriod is the value to restore the lengthTimer once it expires (reaches 0, or trigger event).
    uBYTE lengthPeriod = nr21 & 0b00111111;

    // volumePeriod is the value to restore the volumeTimer once it expires (reaches 0, or trigger event).
    uBYTE volumePeriod = (nr22 & 0b00000111);

    // initialVolume of the channel once a trigger event occurs.
    uBYTE initialVolume = (nr22 & 0b11110000) >> 4;

    if (memRef->TriggerEventCh2()) {

        // The channel is immediately re-enabled if it was disabled.
        ch2Disabled = false;

        // If the length timer is 0 at this point, restore it with 64.
        // This seems to be how the hardware handles the lengthTimer on trigger event,
        // and completely ignores the lengthPeriod value.
        if (ch2LengthTimer == 0)
            ch2LengthTimer = 64;

        // Restore the channel's frequency timer value using NR24 and NR23.
        ch2FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr24, nr23);

        // Restore the volume timer with its period.
        ch2VolumeTimer = volumePeriod;

        // Restore the channel's volume with the initial volume.
        ch2CurrentVolume = initialVolume;
    }

    // First, substract the amount of cpu cycles that have occured from the
    // frequency timer.
    ch2FrequencyTimer -= cycles;

    // If this results in the frequency timer reaching 0, then we
    // reset the timer's value with what is in NR24 (bits 2-0) | NR23 (8 bit value).
    // we also increment the wave pattern pointer when this happens.
    if (ch2FrequencyTimer <= 0) {
        ch2FrequencyTimer = determineSquareWaveFrequencyTimerValue(nr24, nr23) + ch2FrequencyTimer;

        // The pointer value can only be within 0-7, and loops back once it
        // reaches 8.
        ch2WaveDutyPointer = (ch2WaveDutyPointer + 1) % 8;
    }

    // Fetch the raw wave amplitude for channel 2's wave pattern.
    // If the bit at the pointer is '1', then the amplitude is at its maximum value of 255.
    uBYTE wavePattern = determineChannelWavePattern(nr21);
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

    return ch2Amplitude;
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

uBYTE determineChannelWavePattern(uBYTE nrx1) {
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

int determineSquareWaveFrequencyTimerValue(uBYTE hiByte, uBYTE loByte) {
    int frequencyQuotient = ((hiByte & 0x7) << 8) | loByte;

    return (2048 - frequencyQuotient) * 4;
}
