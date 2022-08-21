#include "Apu.hpp"

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
uBYTE computeVolumeModifier(uBYTE currentVolume);
uBYTE determineChannelWavePattern(uBYTE nrx1);

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

    addToBufferTimer = CPU_FREQUENCY / APU_SAMPLE_RATE;

    ch2FrequencyTimer = 0;
    ch2WaveDutyPointer = 0;
    ch1WaveDutyPointer = 0;
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

    flusherRunning = true;
    apuFlusher = std::unique_ptr<std::thread>(new std::thread(&Apu::FlusherRoutine, this));
}

Apu::~Apu() {
    flusherRunning = false;
    loopCv.notify_one();
    if (apuFlusher->joinable()) {
        apuFlusher->join();
    }
    pa_simple_free(audioClient);
}

void Apu::FlusherRoutine() {
    while (flusherRunning) {
        std::unique_lock<std::mutex> bufferLock(loopMtx);
        loopCv.wait(bufferLock);
        FlushBuffer();
        bufferLock.unlock();
    }
}

void Apu::NotifyFlusher() {
    loopCv.notify_one();
}

void Apu::FlushBuffer() {
    int errorCode = 0;

    pa_simple_write(audioClient, buffer, BUFFER_SIZE, &errorCode);
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
    uBYTE nr52 = memRef->rom[NR52];

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

    uBYTE ch1Amplitude = ComputeChannel1Ampltiude();
    uBYTE ch2Amplitude = ComputeChannel2Amplitude();

    // Mix the amplitudes of all channels
    uBYTE sample = ch2Amplitude;

    uBYTE nr50 = memRef->rom[NR50];
    uBYTE nr51 = memRef->rom[NR51];

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

    sample -= volume;

    // TODO: Determine what to do with NR52 bits 0-3

        // Stuff the current sample in the buffer,
    // and play the sound if the buffer is full
    buffer[currentSampleBufferPosition++] = sample;
    if (currentSampleBufferPosition >= BUFFER_SIZE) {
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

        ch1FrequencyTimer = DetermineChannel1FrequencyTimerValue();
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

uBYTE Apu::ComputeChannel2Amplitude() {
    uBYTE ch2Amplitude = 0;

    uBYTE nr21 = memRef->rom[NR21];
    uBYTE nr22 = memRef->rom[NR22];
    uBYTE nr23 = memRef->rom[NR23];
    uBYTE nr24 = memRef->rom[NR24];

    bool lengthEnabled = nr24 & 0b01000000;
    bool volumeDirection = nr22 & 0b00001000;
    uBYTE lengthPeriod = nr21 & 0b00111111;
    uBYTE volumePeriod = (nr22 & 0b00000111);
    uBYTE initialVolume = (nr22 & 0b11110000) >> 4;

    if (nr24 & (1 << 7)) {
        memRef->rom[NR24] = nr24 & ~(1 << 7);

        ch2Disabled = false;

        if (ch2LengthTimer == 0)
            ch2LengthTimer = 64;

        ch2FrequencyTimer = DetermineChannel2FrequencyTimerValue();
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

    // Fetch the raw wave pattern amplitude for channel 2
    uBYTE wavePattern = determineChannelWavePattern(nr21);
    if (wavePattern & (1 << ch2WaveDutyPointer)) {
        ch2Amplitude = 128;
    }
    else {
        ch2Amplitude = 0;
    }

    if (volumeEnvelopeTick) {
        volumeEnvelopeFunction(ch2VolumeTimer, ch2CurrentVolume, volumePeriod, volumeDirection);
    }

    if (memRef->RequiresCh1LengthReload()) {
        ch2LengthTimer = 64 - lengthPeriod;
    }

    if (lengthControlTick && lengthEnabled) {
        lengthFunction(ch2LengthTimer, ch2Disabled);
    }

    ch2Amplitude -= computeVolumeModifier(ch2CurrentVolume);

    if (ch2Disabled) {
        ch2Amplitude = 0;
    }

    return ch2Amplitude;
}

int Apu::DetermineChannel1FrequencyTimerValue() {
    uBYTE hiByte = memRef->rom[NR14] & 0b00000111;
    uBYTE loByte = memRef->rom[NR13];

    int frequencyQuotient = (hiByte << 8) | loByte;

    // These values were pulled from the gbdev pandocs
    // https://gbdev.io/pandocs/Sound_Controller.html#ff19---nr24---channel-2-frequency-hi-data-rw
    return (131072 / (2048 - frequencyQuotient));
}

int Apu::DetermineChannel2FrequencyTimerValue() {
    uBYTE hiByte = memRef->rom[NR24] & 0b00000111;
    uBYTE loByte = memRef->rom[NR23];

    int frequencyQuotient = (hiByte << 8) | loByte;

    // These values were pulled from the gbdev pandocs
    // https://gbdev.io/pandocs/Sound_Controller.html#ff19---nr24---channel-2-frequency-hi-data-rw
    return (131072 / (2048 - frequencyQuotient));
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
    }

    if (envelopeDirection == DECREASING && currentVolume > 0x0) {
        currentVolume--;
    }
}

uBYTE computeVolumeModifier(uBYTE currentVolume) {
    uBYTE volumeModifier = 128;
    if (currentVolume == 0) {
        volumeModifier = 128;
    }
    else if (currentVolume == 0xF) {
        volumeModifier = 0;
    }
    else if (currentVolume > 0 && currentVolume < 0xF) {
        volumeModifier = 128 - ((128 / 0xF) * currentVolume);
    }
    else {
#ifdef FUUGB_DEBUG
        fprintf(stderr, "[APU] invalid current volume modifier computed for channel 2: %d", volumeModifier);
#endif
        exit(EXIT_FAILURE);
    }

    return volumeModifier;
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
