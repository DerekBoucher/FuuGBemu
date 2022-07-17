#ifndef SIDENAV_HPP
#define SIDENAV_HPP

#include "Gameboy.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_memory_editor/imgui_memory_editor.h"

#include <GLFW/glfw3.h>

#define IMGUI_SIZE_X 550

class SideNav {
public:
    SideNav(Gameboy* gbRef);
    ~SideNav();

    bool Init(GLFWwindow* windowRef);
    void Render();
    void Shutdown();

private:
    void renderEmulatorControlsWindow();
    void renderDebuggerWindow();
    void renderDebuggerTabButtons();
    void renderMemoryPane();
    void renderVideoPane();
    void renderAudioPane();

    Gameboy* gbRef;
    MemoryEditor memoryEditor;
    std::string pauseButtonLabels[2] = { "Pause", "Resume" };
    int pauseLabelIdx = 0;
    enum debuggerTab {
        MEMORY,
        VIDEO,
        AUDIO
    };

    debuggerTab selection = MEMORY;
    bool selectedListBox[3] = { true, false, false };
};

#endif
