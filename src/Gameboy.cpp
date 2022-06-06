#include "Gameboy.hpp"

const int CyclesPerFrame = 69905;

Gameboy::Gameboy() {}

Gameboy::~Gameboy() {
    delete apu;
}

Gameboy::Gameboy(uBYTE romData[MAX_CART_SIZE], GLFWwindow* context) {
    // Set up memory
    memory.ReadRom(romData);

    // Set up cpu
    cpu.SetMemory(&memory);

    // Compile shaders
    Shader vertexShader = Shader("src/opengl/shaders/Vertex.shader");
    Shader fragmentShader = Shader("src/opengl/shaders/Fragment.shader");

    // Set up ppu
    ppu.SetContext(context);
    ppu.AttachShaders(vertexShader, fragmentShader);
    ppu.SetMemory(&memory);
    ppu.InitializeGLBuffers();

    // Set up Apu
    apu = new Apu(&memory);
}

void Gameboy::Wait() {
    std::unique_lock<std::mutex> pauseLock(mtx);
    cv.wait(pauseLock);
    pauseLock.unlock();
    pause = false;
}

void Gameboy::Start() {
    running = true;
    thread = std::unique_ptr<std::thread>(new std::thread(&Gameboy::Run, this));
}

void Gameboy::Stop() {
    running = false;
    if (thread->joinable())
        thread->join();
}

void Gameboy::SkipBootRom() {
    cpu.SetPostBootRomState();
    memory.SetPostBootRomState();
}

void Gameboy::Run() {
    // Set the context as current on this thread
    ppu.BindContext();

#ifdef FUUGB_DEBUG
    int frames = 0;
    double lastTimestamp = glfwGetTime();
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
                Wait();
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

        // Render screen
        ppu.Render();

#ifdef FUUGB_DEBUG
        frames++;
        double currentTime = glfwGetTime();
        if (currentTime - lastTimestamp >= 1.0) {
            std::cout << "FPS: " << frames << std::endl;
            frames = 0;
            lastTimestamp = glfwGetTime();
        }
#endif
    }
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
