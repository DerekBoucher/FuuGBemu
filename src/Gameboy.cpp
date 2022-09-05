#include "Gameboy.hpp"

const int CyclesPerFrame = CPU_FREQUENCY_HZ / 60;
const double singleFramePeriod = 1.0 / 60.0;

Gameboy::Gameboy() {}

Gameboy::~Gameboy() {}

Gameboy::Gameboy(uBYTE romData[MAX_CART_SIZE], GLFWwindow* context) {
    memory.ReadRom(romData);

    cpu.SetMemory(&memory);

    Shader vertexShader = Shader("src/opengl/shaders/Vertex.shader");
    Shader fragmentShader = Shader("src/opengl/shaders/Fragment.shader");

    ppu.SetContext(context);
    ppu.AttachShaders(vertexShader, fragmentShader);
    ppu.SetMemory(&memory);
    ppu.InitializeGLBuffers();

    apu = std::unique_ptr<Apu>(new Apu(&memory));

    requireRender = false;
}

void Gameboy::WaitRender() {
    std::unique_lock<std::mutex> lock(mtx);
    renderingCV.wait(lock);
    lock.unlock();
}

void Gameboy::WaitResume() {
    std::unique_lock<std::mutex> lock(mtx);
    pauseCV.wait(lock);
    lock.unlock();
    pause = false;
}

void Gameboy::Pause() {
    pause = true;
}

void Gameboy::Resume() {
    pauseCV.notify_one();
}

void Gameboy::Start() {
    running = true;
    thread = std::unique_ptr<std::thread>(new std::thread(&Gameboy::Run, this));
}

void Gameboy::Stop() {
    running = false;
    while (!finished) {
        renderingCV.notify_one();
        pauseCV.notify_one();
    }
    thread->join();
}

void Gameboy::SkipBootRom() {
    cpu.SetPostBootRomState();
    memory.SetPostBootRomState();
}

void Gameboy::Run() {
    // For FPS Capping
    double lastFrameTimeStamp = glfwGetTime();

#ifdef FUUGB_DEBUG
    int frames = 0;
    double lastFPSCounterTimestamp = glfwGetTime();
#endif

    // Main gameboy loop
    while (running) {

        // We emulate the gameboy by keeping track of the clock cycles
        // that the cpu has executed. The gameboy's ppu refreshes the display
        // 60 times a second. The inverse of this frequency means that we need to 
        // render the screen every 66905 clock cycles.
        int cyclesThisUpdate = 0;

        while (cyclesThisUpdate <= CyclesPerFrame) {
            int cycles = 0;

            // If gameboy is paused, pause the thread
            if (pause) {
                WaitResume();
            }

            // If cpu is halted, halt it
            if (cpu.Halted) {
                cycles = 4;
                memory.UpdateTimers(cycles);
                cpu.Halt();
            }
            else {
                cycles = cpu.ExecuteNextOpCode();
            }

            // Update components
            cyclesThisUpdate += cycles;
            ppu.UpdateGraphics(cycles);
            memory.UpdateDmaCycles(cycles);
            apu->UpdateSoundRegisters(cycles);

            // Process interrupts
            if (!cpu.Halted) {
                cpu.CheckInterupts();
            }
        }

        // Not the fanciest solution, but here we wait
        // until at least a singleFramePeriod of time has
        // passed before rendering the next frame.
        // This is to avoid some machines with faster hardware
        // to have the gameboy run too quickly.
        // (This caps the emulation at ~60FPS)
        double currentTime = glfwGetTime();
        while (currentTime - lastFrameTimeStamp < singleFramePeriod)
            currentTime = glfwGetTime();

        lastFrameTimeStamp = glfwGetTime();

        // Ask the main thread to perform the rendering
        requireRender = true;
        WaitRender();

#ifdef FUUGB_DEBUG
        frames++;
        if (currentTime - lastFPSCounterTimestamp >= 1.0) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastFPSCounterTimestamp = glfwGetTime();
        }
#endif
    }

    finished = true;
}

void Gameboy::HandleKeyboardInput(int key, int scancode, int action, int modBits) {

    // Ignore any keyboard action that is not PRESSED
    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    // The emulator uses a custom scheme that the memory module
    // translates to what the original gameboy hardware would expect
    // Bit 7 - Down
    // Bit 6 - Up
    // Bit 5 - Left
    // Bit 4 - Right
    // Bit 3 - Start
    // Bit 2 - Select
    // Bit 1 - B
    // Bit 0 - A

    if (action == GLFW_RELEASE) {
        memory.joypadBuffer = 0xFF;
        return;
    }

    if (key == GLFW_KEY_DOWN) {
        memory.joypadBuffer &= ~(1 << 7);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_UP) {
        memory.joypadBuffer &= ~(1 << 6);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_LEFT) {
        memory.joypadBuffer &= ~(1 << 5);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_RIGHT) {
        memory.joypadBuffer &= ~(1 << 4);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_C) {
        memory.joypadBuffer &= ~(1 << 3);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_V) {
        memory.joypadBuffer &= ~(1 << 2);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_X) {
        memory.joypadBuffer &= ~(1 << 1);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }

    if (key == GLFW_KEY_Z) {
        memory.joypadBuffer &= ~(1 << 0);
        memory.RequestInterupt(CONTROL_INT);
        return;
    }
}

bool Gameboy::RequiresRender() {
    return requireRender;
}

void Gameboy::Render() {
    ppu.Render();
    requireRender = false;
    renderingCV.notify_one();
}
