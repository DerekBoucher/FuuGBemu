#include "GUI.h"

wxBEGIN_EVENT_TABLE(GUI, wxFrame)
    EVT_MENU(wxID_OPEN, GUI::OnClickOpen)
        EVT_MENU(wxID_EXIT, GUI::OnClickExit)
            EVT_MENU(wxID_CUSTOM_DEBUGGER, GUI::OnClickDebugger)
                EVT_CLOSE(GUI::OnClose)
                    wxEND_EVENT_TABLE();

GUI::GUI() : wxFrame(NULL, wxID_ANY, wxT("FuuGBemu"), wxDefaultPosition, wxSize(NATIVE_SIZE_X * SCALE_FACTOR, NATIVE_SIZE_Y * SCALE_FACTOR), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER))
{
    wxMenuBar *menuBar = new wxMenuBar();
    wxMenu *fileMenu = new wxMenu();
    wxMenu *viewMenu = new wxMenu();

    fileMenu->Append(wxID_OPEN, wxT("&Open"));
    fileMenu->Append(wxID_EXIT, wxT("&Exit"));
    viewMenu->Append(wxID_CUSTOM_DEBUGGER, wxT("&Debugger"));

    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(viewMenu, wxT("&View"));

    SetMenuBar(menuBar);

    SetSize(wxSize(GetSize().GetX(), GetSize().GetY() + menuBar->GetSize().GetY() + 10));
    Centre();

    gameboyScreen = new wxWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
}

GUI::~GUI()
{
}

void GUI::OnClickOpen(wxCommandEvent &e)
{
    if (gameboy != nullptr)
    {
        gameboy->Pause();
    }
    wxFileDialog openfileDialog(this, wxT("Select a ROM file"), "./real", "", ".gb ROM files (*.gb)|*.gb", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openfileDialog.ShowModal() == wxID_OK)
    {
        wxPrintf("Loading %s...\n", openfileDialog.GetPath());
        FILE *romfile = fopen(openfileDialog.GetPath(), "r");
        Cartridge *cart = new Cartridge(romfile);

        if (gameboy != nullptr)
        {
            gameboy->Resume();
            gameboy->Stop();
            delete gameboy;
        }
        gameboy = new Gameboy(gameboyScreen, cart);
    }
    else if (gameboy != nullptr)
    {
        gameboy->Resume();
    }
    e.Skip();
}

void GUI::OnClickExit(wxCommandEvent &e)
{
    Close();
    e.Skip();
}

void GUI::OnClose(wxCloseEvent &e)
{
    if (gameboy != NULL)
    {
        gameboy->Resume();
        gameboy->Stop();
        delete gameboy;
    }
    Destroy();
    SDL_Quit();
    e.Skip();
}

void GUI::OnClickDebugger(wxCommandEvent &e)
{
    if (gameboy == nullptr)
    {
        wxMessageBox(wxT("No ROM is currently loaded into the emulator. Please load a ROM and try again."), wxT("No ROM Selected"), 5L, this);
        e.Skip();
        return;
    }
    gameboy->Pause();
    Disable();
    debugger = new Debugger(this, gameboy, gameboy->GetMemory(), gameboy->GetCartridge());
    debugger->Show();
    e.Skip();
}