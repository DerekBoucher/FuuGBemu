#ifndef APU_HPP
#define APU_HPP

#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "Memory.hpp"

#define NR10 0xFF10 // Channel 1 Sweep Register
#define NR11 0xFF11 // Channel 1 Sound length/Wave Pattern duty
#define NR12 0xFF12 // Channel 1 Volume Envelope
#define NR13 0xFF13 // Channel 1 Frequency lo
#define NR14 0xFF14 // Channel 1 Frequency Hi

#define NR21 0xFF16 // Channel 2 Sound length/Wave Pattern duty
#define NR22 0xFF17 // Channel 2 Volume Envelope
#define NR23 0xFF18 // Channel 2 Frequency lo
#define NR24 0xFF19 // Channel 2 Frequency hi

#define NR30 0xFF1A // Channel 3 Sound On/Off
#define NR31 0xFF1B // Channel 3 Sound Length
#define NR32 0xFF1C // Channel 3 Select Output Level
#define NR33 0xFF1D // Channel 3 Frequency Lo
#define NR34 0xFF1E // Channel 3 Frequency Hi

#define NR41 0xFF20 // Channel 4 Sound Length
#define NR42 0xFF21 // Channel 4 Volume Envelope
#define NR43 0xFF22 // Channel 4 Polynomial Counter
#define NR44 0xFF23 // Channel 4 Counter / consecutive

#define NR50 0xFF24 // Channel Control / On-Off / Volume
#define NR51 0xFF25 // Selection of output channel
#define NR52 0xFF26 // Sound On/Off

#define DUTY1 0b00000001 // 12.5% duty cycle
#define DUTY2 0b00000011 // 25% duty cycle
#define DUTY3 0b00001111 // 50% duty cycle
#define DUTY4 0b00111111 // 75% duty cycle

#define WAVE_RAM 0xFF30

#define SQUARE_WAVE_MAX_FREQUENCY_HZ 2048
#define FRAME_SEQUENCER_FREQUENCY_HZ 512
#define AUDIO_SAMPLING_FREQUENCY_HZ 48000
#define CPU_FREQUENCY_HZ 4194304

#define AUDIO_BUFFER_SIZE 1024

#define INCREASING true
#define DECREASING false

class Apu {
    friend class SideNav;

public:
    Apu();
    ~Apu();

    void UpdateSound(int cycles);
    void SetMemory(Memory* memRef);

private:
    void FlushBuffer();
    void UpdateFrameSequencer(int cycles);
    uBYTE ComputeChannel1Amplitude(int cycles);
    uBYTE ComputeChannel2Amplitude(int cycles);
    uBYTE ComputeChannel3Amplitude(int cycles);

    // Emulator specific channel toggles
    bool debuggerCh1Toggle;
    bool debuggerCh2Toggle;

    Memory* memRef;

    // Audio buffer flusher thread routine
    bool flusherRunning;
    std::mutex flusherMtx;
    std::unique_ptr<std::thread> apuFlusher;
    std::unique_lock<std::mutex> flusherLock;
    std::condition_variable flusherCv;

    // Audio sample buffer variables
    std::mutex bufferLock;
    uBYTE audioBuffer[AUDIO_BUFFER_SIZE];
    int currentSampleBufferPosition;
    int addToBufferTimer;

    // Frame sequencer variables
    int frameSequencerTimer;
    int frameSequencerStep;
    bool lengthControlTick;
    bool volumeEnvelopeTick;
    bool sweepTick;

    // Square Channel 1 variables
    bool ch1Disabled;
    int ch1ShadowFrequency;
    int ch1FrequencyTimer;
    int ch1VolumeTimer;
    uBYTE ch1CurrentVolume;
    uBYTE ch1SweepTimer;
    uBYTE ch1LengthTimer;
    uBYTE ch1WaveDutyPointer;

    // Square Channel 2 variables
    bool ch2Disabled;
    int ch2VolumeTimer;
    int ch2FrequencyTimer;
    uBYTE ch2LengthTimer;
    uBYTE ch2WaveDutyPointer;
    uBYTE ch2CurrentVolume;

    // Wave RAM Channel 3 variables
    bool ch3Disabled;
    int ch3FrequencyTimer;
    uBYTE ch3LengthTimer;
    uBYTE ch3WavePointer;
    uBYTE ch3SamplePointer;

#ifdef FUUGB_SYSTEM_LINUX
    pa_simple* audioClient;
#endif

};

#endif
