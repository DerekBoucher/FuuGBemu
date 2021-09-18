#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string.h>
#include <fstream>
#include <csignal>

#include "Gameboy.hpp"

#define NATIVE_SIZE_X 160
#define NATIVE_SIZE_Y 144
#define SCALE 5

GLFWwindow* window;
Gameboy* gameboy;

static void GLAPIENTRY MessageCallback( GLenum source,
                    GLenum type,
                    GLuint id,
                    GLenum severity,
                    GLsizei length,
                    const GLchar* message,
                    const void* userParam) {
                        if (type == GL_DEBUG_TYPE_ERROR) {
                            fprintf( stderr, "** GL ERROR **: type = 0x%x, severity = 0x%x, message = %s\n",
                                type, severity, message); 
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

void printUsage() {
    fprintf(stdout, "FuuGBemu\n");
    fprintf(stdout, "Usage:\n");
    fprintf(stdout, "\tFuuGBemu [OPTIONS] <rom path>\n");
    fprintf(stdout, "Options:\n");
}

int main(int argc, char** argv) {

    // If user did not specify rom path, remind them to :)
    if (argc < 2) {
        fprintf(stderr, "missing rom path\n");
        printUsage();
        return EXIT_FAILURE;
    }

    std::fstream romFile(argv[1], std::ios::in | std::ios::binary);
    if (!romFile.good()) {
        fprintf(stderr, "error reading rom file: %s\n", strerror(errno));
        printUsage();
        return EXIT_FAILURE;
    }

    // Read rom data
    char romData[MAX_CART_SIZE];
    romFile.read(romData, MAX_CART_SIZE);

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create window
    window = glfwCreateWindow(NATIVE_SIZE_X * SCALE,
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
    gameboy = new Gameboy((uBYTE*)romData, window);

    // Set viewport
    glViewport(0, 0, NATIVE_SIZE_X * SCALE, NATIVE_SIZE_Y * SCALE);

    // Clear the background to white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    // Before we start the gameboy, unbind the context from the
    // main thread. The gameboy thread will handle it from here
    glfwMakeContextCurrent(NULL);

    // Set keyboard handler
    glfwSetKeyCallback(window, keyboardHandler);

    // Create a gameboy instance, then start it.
    gameboy->Start();

    // Here all we want the main thread to do
    // is handle window events.
    // The gameboy will handle swapping of buffers
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    gameboy->Stop();

    delete gameboy;
    glfwTerminate();

    return EXIT_SUCCESS;
}
