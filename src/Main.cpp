#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string.h>
#include <fstream>
#include <csignal>

#include "SideNav.hpp"
#include "Gameboy.hpp"
#include "Apu.hpp"

#define NATIVE_SIZE_X 160
#define NATIVE_SIZE_Y 144
#define SCALE 5

using namespace std;

GLFWwindow* window;
Gameboy* gameboy;

bool skipBootRom = false;
bool imguiActive = true;
bool imguiDisable = false;
std::string romPath = "";

#ifdef FUUGB_DEBUG
static void GLAPIENTRY MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) {
    if (type == GL_DEBUG_TYPE_ERROR) {
        fprintf(stderr, "** GL ERROR **: type = 0x%x, severity = 0x%x, message = %s\n",
            type, severity, message);
    }
}
#endif

void printUsage() {
    fprintf(stdout, "FuuGBemu\n");
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "\tFuuGBemu [OPTIONS] <rom path>\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "\t--skip-boot-rom\t\tSkips the boot rom and enters the game code immediately.\n");
}

void parseArguments(int argc, char** argv) {
    // No arguments passed
    if (argc < 2) {
        fprintf(stderr, "missing arguments\n");
        printUsage();
        exit(EXIT_FAILURE);
    }

    // Process all the arguments
    for (int i = 1; i < argc; i++) {
        std::string token = argv[i];

        if (token.find("--skip-boot-rom") != std::string::npos) {
            skipBootRom = true;
            continue;
        }

        // If the user entered another option, it is unrecognized.
        if (token.find("--") != std::string::npos) {
            fprintf(stderr, "invalid option passed.\n");
            printUsage();
            exit(EXIT_FAILURE);
        }

        // If no strCmps worked prior to reaching this point,
        // the expected argument is the rom path.
        romPath = token;
    }
}

void signalHandler(int signal) {
    fprintf(stdout, "caught interrupt signal, terminating.\n");
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void keyboardHandler(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (gameboy != NULL)
        gameboy->HandleKeyboardInput(key, scancode, action, mods);
}

int main(int argc, char** argv) {

    parseArguments(argc, argv);

    std::fstream romFile(romPath, std::ios::in | std::ios::binary);
    if (!romFile.good()) {
        fprintf(stderr, "error reading rom file: %s\n", strerror(errno));
        printUsage();
        return EXIT_FAILURE;
    }

    // Read rom data
    uBYTE* romData = new uBYTE[MAX_CART_SIZE];
    romFile.read((char*)romData, MAX_CART_SIZE);

    // Set interrupt signal handler
    signal(SIGINT, signalHandler);

    // Initialize glfw
    if (!glfwInit()) {
        const char* errMsg[1024];
        glfwGetError(errMsg);
        fprintf(stderr, "could not initialize glfw: %s\n", *errMsg);
        return EXIT_FAILURE;
    }

    // Use OpenGL 3.3 Core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create window
    window = glfwCreateWindow((NATIVE_SIZE_X * SCALE) + IMGUI_SIZE_X,
        NATIVE_SIZE_Y * SCALE,
        "FuuGBemu",
        NULL,
        NULL);

    // If something went wrong, report the error
    if (!window) {
        const char* errMsg[1024];
        glfwGetError(errMsg);
        fprintf(stderr, "could not create glfw window: %s\n", *errMsg);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Make the created window as the current context
    glfwMakeContextCurrent(window);

    GLenum glewInitCode = glewInit();
    if (glewInitCode != GLEW_OK) {
        fprintf(stderr, "could not initialize glew: %s\n", glewGetErrorString(glewInitCode));
        glfwTerminate();
        return EXIT_FAILURE;
    }

#ifdef FUUGB_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);
#endif

    // Create a gameboy instance
    gameboy = new Gameboy(romData, window);

    SideNav sideNav = SideNav(gameboy);
    if (!sideNav.Init(window))
        return EXIT_FAILURE;

    // If the user wants to skip boot rom, set the state on the gameboy
    if (skipBootRom) {
        gameboy->SkipBootRom();
    }

    // Set viewport
    glViewport(0, 0, NATIVE_SIZE_X * SCALE, NATIVE_SIZE_Y * SCALE);

    // Clear the background to white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // Set keyboard handler
    glfwSetKeyCallback(window, keyboardHandler);

    // Start gameboy
    gameboy->Start();

    // Main program loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        sideNav.Render();

        if (gameboy->RequiresRender()) {
            gameboy->Render();
        }

        glfwSwapBuffers(window);
    }

    // Clean up
    gameboy->Stop();
    sideNav.Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    delete gameboy;
    delete[] romData;

    return EXIT_SUCCESS;
}
