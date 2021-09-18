#include "Gameboy.hpp"

const int CyclesPerFrame = 69905;

Gameboy::Gameboy() {}

Gameboy::~Gameboy() {}

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

void Gameboy::Run() {
    
    // Set the context as current on this thread
    ppu.BindContext();

    // Main gameboy loop
    while(running) {
        
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
            } else {
                cycles = cpu.ExecuteNextOpCode();
            }

            // Update components
            cyclesThisUpdate += cycles;
            ppu.UpdateGraphics(cycles);
            memory.UpdateDmaCycles(cycles);

            // Process interrupts
            if (!cpu.Halted) {
                cpu.CheckInterupts();
            }
        }

        // Render screen
        ppu.Render();
    }
}

void Gameboy::HandleKeyboardInput(int key, int scancode, int action, int modBits) {
    
    // Ignore any keyboard action that is not PRESSED
    if (action != GLFW_PRESS && action != GLFW_RELEASE)
        return;

    if (action == GLFW_RELEASE) {
        memory.rom[JOYPAD_INPUT_REG] = 0xFF;
        return;
    }

    // User pressed either DOWN or START
    if (key == GLFW_KEY_DOWN || key == GLFW_KEY_Z) {
        memory.rom[JOYPAD_INPUT_REG] &= ~(1 << 3);
        memory.RequestInterupt(CONTROL_INT);
    }

    // User pressed either UP or SELECT
    if (key == GLFW_KEY_UP || key == GLFW_KEY_X) {
        memory.rom[JOYPAD_INPUT_REG] &= ~(1 << 2);
        memory.RequestInterupt(CONTROL_INT);
    }

    // User pressed either LEFT or B
    if (key == GLFW_KEY_LEFT || key == GLFW_KEY_S) {
        memory.rom[JOYPAD_INPUT_REG] &= ~(1 << 1);
        memory.RequestInterupt(CONTROL_INT);
    }

    // User pressed either RIGHT or A
    if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_A) {
        memory.rom[JOYPAD_INPUT_REG] &= ~(1 << 0);
        memory.RequestInterupt(CONTROL_INT);
    }
}
