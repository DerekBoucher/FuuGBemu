#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "Ppu.hpp"
#include "Memory.hpp"
#include "Cpu.hpp"

#include <thread>
#include <iostream>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <mutex>
#include <condition_variable>

class Gameboy {

public:
    Gameboy();
    Gameboy(uBYTE romData[MAX_CART_SIZE], GLFWwindow* context);
    ~Gameboy();

    void Start();
    void Stop();
    void Pause();
    void Resume();

    void HandleKeyboardInput(int key, int scancode, int action, int modBits);

private:
    void Wait();

    Cpu cpu;
    Ppu ppu;
    Memory memory;
    bool running;
    std::mutex mtx;
    std::condition_variable cv;
    bool pause;

    std::unique_ptr<std::thread> thread;

    void Run();
};

#endif
