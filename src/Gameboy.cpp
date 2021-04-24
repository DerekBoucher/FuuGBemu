#include "Gameboy.h"
using namespace std::chrono;

Gameboy::Gameboy(wxWindow *screen, Cartridge *c)
{
    running = true;
    cart = c;
    memoryUnit = new Memory(c);
    ppuUnit = new PPU(screen, this->memoryUnit);
    cpuUnit = new CPU(this->memoryUnit);
    pause = false;
    thread = new std::thread(&Gameboy::Run, this);
}

Gameboy::~Gameboy()
{
    delete thread;
    delete ppuUnit;
    delete memoryUnit;
    delete cpuUnit;
}

void Gameboy::Run()
{
    const int MaxCycles = 69905;

    while (running)
    {
        int cyclesThisUpdate = 0;

        auto frameStart = CurrentTimestamp();

        while (cyclesThisUpdate <= MaxCycles)
        {
            int cycles = 0;

            if (pause)
            {
                wait();
            }

            if (cpuUnit->Halted)
            {
                cycles = 4;
                memoryUnit->UpdateTimers(cycles);
                cpuUnit->Halt();
            }
            else
            {
                cycles = cpuUnit->ExecuteNextOpCode();
            }

            cyclesThisUpdate += cycles;
            ppuUnit->UpdateGraphics(cycles);
            memoryUnit->UpdateDmaCycles(cycles);

            if (!cpuUnit->Halted)
            {
                cpuUnit->CheckInterupts();
            }
        }

        WaitForFrame(frameStart);

        ppuUnit->RenderScreen();
    }
}

void Gameboy::Pause()
{
    pause = true;
}

void Gameboy::Resume()
{
    cv.notify_all();
}

void Gameboy::Stop()
{
    running = false;
    thread->join();
}

void Gameboy::wait()
{
    std::unique_lock<std::mutex> pauseLock(mtx);
    cv.wait(pauseLock);
    pauseLock.unlock();
    pause = false;
}

Memory *Gameboy::GetMemory()
{
    return memoryUnit;
}

Cartridge *Gameboy::GetCartridge()
{
    return cart;
}

Timestamp Gameboy::CurrentTimestamp()
{
    return std::chrono::high_resolution_clock::now();
}

void Gameboy::WaitForFrame(Timestamp start)
{
    Timestamp end = CurrentTimestamp();

    duration<double> elapsedTime = duration_cast<std::chrono::duration<double>>(end - start);

    if (elapsedTime < milliseconds(FRAME_TIME_MS))
    {
        std::this_thread::sleep_for(milliseconds(FRAME_TIME_MS) - elapsedTime);
    }
}