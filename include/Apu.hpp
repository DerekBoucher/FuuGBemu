#ifndef APU_HPP
#define APU_HPP

#include <pulse/simple.h>
#include <pulse/error.h>
#include <stdio.h>
#include <stdlib.h>

#include "Memory.hpp"

#define BUFFER 1024

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
#define DUTY4 0b11111100 // 75% duty cycle

#define FRAME_SEQUENCER_CYCLES 8192
#define CH2_FREQUENCY_CYCLES 2048
#define CH1_SWEEP_CYCLES 2048
#define MAX_FREQUENCY 2048

#define FRAME_SEQUENCE_VOLUME_CLOCK 7
#define FRAME_SEQUENCE_SWEEP_CLOCK_2 2
#define FRAME_SEQUENCE_SWEEP_CLOCK_6 6
#define CPU_FREQUENCY 4194304
#define APU_SAMEPLE_RATE 44100

class Apu {

public:
    Apu();
    Apu(Memory* memRef);
    ~Apu();

    void PlaySample();
    void WaitFlush();
    void ProcessChannel2();
    void UpdateSoundRegisters(int cycles);
    void AddSampleToBuffer(int cycles);
private:
    void FlushBuffer();

    Memory* memRef;
    uBYTE channel2DutyPosition;
    int ch2FrequencyTimer;
    int ch4FrequencyTimer;

    uWORD ch4LFSR;
    int frameSequencerTimer;
    uBYTE frameSequencerStep;

    int ch1EnvelopeTimer;
    int ch2EnvelopeTimer;
    int ch4EnvelopeTimer;

    int ch1EnvelopeVolume;
    int ch2EnvelopeVolume;
    int ch4EnvelopeVolume;

    bool sweepEnabled;
    int shadowFrequency;
    int sweepTimer;
    int currentFrequency;

    int ch1LengthTimer;
    int ch2LengthTimer;
    int ch3LengthTimer;
    int ch4LengthTimer;

    uBYTE leftSample;
    uBYTE rightSample;

    int addToBufferTimer;
    int currentPositionBuffer;
    int currentBuffer;

#ifdef FUUGB_SYSTEM_LINUX
    pa_simple* audioClient;
#endif

    uint8_t buffer[2][BUFFER];

};

#endif
