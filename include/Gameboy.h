#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "Defines.h"
#include "CPU.h"
#include "PPU.h"

#include <thread>
#include <chrono>
#include <condition_variable>

typedef std::chrono::high_resolution_clock::time_point Timestamp;

class Gameboy
{
public:
    Gameboy(wxWindow *, Cartridge *);
    Gameboy(Gameboy &) = delete;
    virtual ~Gameboy();

    void Pause();
    void Resume();
    void Run();
    void Stop();

    Memory *GetMemory();
    Cartridge *GetCartridge();

private:
    CPU *cpuUnit;
    Cartridge *cart;
    Memory *memoryUnit;
    PPU *ppuUnit;
    std::thread *thread;
    std::mutex mtx;
    std::condition_variable cv;
    bool pause;
    bool running;

    Timestamp CurrentTimestamp();
    void WaitForFrame(Timestamp);

    void wait();
};

#endif