#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "Ppu.hpp"
#include "Memory.hpp"
#include "Cpu.hpp"
#include "Apu.hpp"

#include <thread>
#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <condition_variable>

class Gameboy {

    friend class SideNav;

public:
    Gameboy();
    Gameboy(uBYTE romData[MAX_CART_SIZE], GLFWwindow* context);
    ~Gameboy();

    void Start();
    void Stop();
    void Pause();
    void Resume();
    void SkipBootRom();
    void Render();

    bool RequiresRender();
    void HandleKeyboardInput(int key, int scancode, int action, int modBits);

private:
    void WaitRender();
    void WaitResume();

    Cpu cpu;
    Ppu ppu;
    Memory memory;

    bool running;
    bool pause;
    bool requireRender;
    bool finished;

    std::mutex mtx;
    std::condition_variable renderingCV;
    std::condition_variable pauseCV;

    std::unique_ptr<Apu> apu;
    std::unique_ptr<std::thread> thread;

    void Run();
};

#endif
