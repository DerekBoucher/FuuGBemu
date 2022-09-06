#include "SideNav.hpp"

const ImVec4 tabSelectedColor = ImVec4(0.0f, 0.0f, 200.0f, 255.0f);

SideNav::SideNav(Gameboy* gbRef) {
    this->gbRef = gbRef;
}

SideNav::~SideNav() {}

bool SideNav::Init(GLFWwindow* windowRef) {
    bool initResult;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Global Styling
    ImGui::StyleColorsDark();
    initResult = ImGui_ImplGlfw_InitForOpenGL(windowRef, true);
    if (!initResult) {
        return initResult;
    }
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

    ImGui::Text("PC: %04X", gbRef->cpu.PC);
    ImGui::Text("SP: %04X", gbRef->cpu.SP);
    ImGui::Text("A: %02X", gbRef->cpu.AF.hi); ImGui::SameLine(); ImGui::Text("F: %02X", gbRef->cpu.AF.lo);
    ImGui::Text("B: %02X", gbRef->cpu.BC.hi); ImGui::SameLine(); ImGui::Text("C: %02X", gbRef->cpu.BC.lo);
    ImGui::Text("D: %02X", gbRef->cpu.DE.hi); ImGui::SameLine(); ImGui::Text("E: %02X", gbRef->cpu.DE.lo);
    ImGui::Text("H: %02X", gbRef->cpu.HL.hi); ImGui::SameLine(); ImGui::Text("L: %02X", gbRef->cpu.HL.lo);

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
    if (ImGui::CollapsingHeader("Cartridge ROM")) {
        memoryEditor.DrawContents(gbRef->memory.cartridge, sizeof(gbRef->memory.cartridge));
    }
    if (ImGui::CollapsingHeader("Video RAM")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x2000, 0x8000);
    }
    if (ImGui::CollapsingHeader("Work RAM 0")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x1000, 0xC000);
    }
    if (ImGui::CollapsingHeader("Work RAM 1")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x1000, 0xD000);
    }
    if (ImGui::CollapsingHeader("Sprite Attribute Table")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x100, 0xFE00);
    }
    if (ImGui::CollapsingHeader("I/O Registers")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x80, 0xFF00);
    }
    if (ImGui::CollapsingHeader("High RAM")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x7F, 0xFF80);
    }
    if (ImGui::CollapsingHeader("Interrupt Enable Register")) {
        memoryEditor.DrawContents(gbRef->memory.rom, 0x1, 0xFFFF);
    }
}

void SideNav::renderVideoPane() {
    // TODO
}

void SideNav::renderAudioPane() {
    ImGui::Text("Sound channel toggles:");
    ImGui::Checkbox("Channel 1", &gbRef->apu->debuggerCh1Toggle);
    ImGui::Checkbox("Channel 2", &gbRef->apu->debuggerCh2Toggle);
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

    if (requiresStylePopping) {
        ImGui::PopStyleColor();
        requiresStylePopping = false;
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
