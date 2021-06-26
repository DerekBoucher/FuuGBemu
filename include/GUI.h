#ifndef GUI_H
#define GUI_H

#include "Defines.h"
#include "Gameboy.h"
#include "Debugger.h"

#include <wx/frame.h>

class GUI : public wxFrame
{

public:
    GUI();
    ~GUI();

    void OnKeyDown(wxKeyEvent &);

    wxDECLARE_EVENT_TABLE();

private:
    Debugger *debugger = nullptr;
    wxWindow *gameboyScreen = nullptr;
    Gameboy *gameboy = nullptr;
    void OnClose(wxCloseEvent &);
    void OnClickOpen(wxCommandEvent &);
    void OnClickExit(wxCommandEvent &);
    void OnClickDebugger(wxCommandEvent &);
};

#endif