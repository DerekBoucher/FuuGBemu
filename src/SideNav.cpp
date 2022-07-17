#include "SideNav.hpp"

const ImVec4 tabSelectedColor = ImVec4(83.0f, 132.0f, 193.0f, 255.0f);

SideNav::SideNav(Gameboy* gbRef) {
    this->gbRef = gbRef;

}

SideNav::~SideNav() {}

bool SideNav::Init(GLFWwindow* windowRef) {
    bool initResult;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Global Styling
    ImGui::StyleColorsLight();
    // ImGuiStyle& style = ImGui::GetStyle();
    initResult = ImGui_ImplGlfw_InitForOpenGL(windowRef, true);
    initResult = ImGui_ImplOpenGL3_Init("#version 330");

    return initResult;
}

void SideNav::Render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderEmulatorControlsWindow();
    renderDebuggerWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SideNav::renderEmulatorControlsWindow() {
    ImGui::SetNextWindowPos(ImVec2(NATIVE_SIZE_X * SCALE, 0));
    ImGui::SetNextWindowSize(ImVec2(IMGUI_SIZE_X, 200));
    ImGui::Begin("Emulator Controls", NULL, ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoNav);

    if (ImGui::Button(pauseButtonLabels[pauseLabelIdx].c_str())) {
        if (pauseButtonLabels[pauseLabelIdx] == "Pause")
            gbRef->Pause();

        if (pauseButtonLabels[pauseLabelIdx] == "Resume")
            gbRef->Resume();

        pauseLabelIdx = (pauseLabelIdx + 1) % 2;
    }

    ImGui::End();
}

void SideNav::renderDebuggerWindow() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    ImGui::SetNextWindowPos(ImVec2(NATIVE_SIZE_X * SCALE, 200));
    ImGui::SetNextWindowSize(ImVec2(IMGUI_SIZE_X, (NATIVE_SIZE_Y * SCALE) - 200));
    ImGui::Begin("null", NULL, ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoNav);

    // Render the tab buttons first
    renderDebuggerTabButtons();

    // Switch statement to determine which tab pane to render
    switch (selection) {
    case VIDEO:
        renderVideoPane();
        break;
    case AUDIO:
        renderAudioPane();
        break;
    case MEMORY:
    default:
        renderMemoryPane();
        break;
    }

    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(2);

    ImGui::End();
}

void SideNav::renderMemoryPane() {
    if (ImGui::CollapsingHeader("Video RAM")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x2000, 0x8000);
    }
    if (ImGui::CollapsingHeader("Cartridge ROM 0")) {
        memoryEditor.DrawContents(gbRef->memory.cartridge, 0x4000);
    }
    if (ImGui::CollapsingHeader("Cartridge ROM n")) {
        ImGui::Text("Currently mapped ROM bank: %d", gbRef->memory.currentRomBank);
        memoryEditor.DrawContents(gbRef->memory.cartridge, 0x4000, 0x4000 + (gbRef->memory.currentRomBank * 0x4000));
    }
}

void SideNav::renderVideoPane() {
    // TODO
}

void SideNav::renderAudioPane() {
    // TODO
}

void SideNav::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void SideNav::renderDebuggerTabButtons() {
    bool requiresStylePopping = false;

    if (selection == MEMORY) {
        ImGui::PushStyleColor(ImGuiCol_Button, tabSelectedColor);
        requiresStylePopping = true;
    }

    if (ImGui::Button("Memory", ImVec2(IMGUI_SIZE_X / 3, 20))) {
        selection = MEMORY;
    }

    if (requiresStylePopping) {
        ImGui::PopStyleColor();
        requiresStylePopping = false;
    }

    if (selection == VIDEO) {
        ImGui::PushStyleColor(ImGuiCol_Button, tabSelectedColor);
        requiresStylePopping = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Video", ImVec2(IMGUI_SIZE_X / 3, 20))) {
        selection = VIDEO;
    }

    if (selection == AUDIO) {
        ImGui::PushStyleColor(ImGuiCol_Button, tabSelectedColor);
        requiresStylePopping = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Audio", ImVec2(IMGUI_SIZE_X / 3, 20))) {
        selection = AUDIO;
    }

    if (requiresStylePopping) {
        ImGui::PopStyleColor();
        requiresStylePopping = false;
    }
}
