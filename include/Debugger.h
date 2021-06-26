#ifndef DEBUGGER_H
#define DEBUUGER_H

#include "Defines.h"
#include "Memory.h"
#include "Cartridge.h"
#include "Gameboy.h"
#include "MemoryViewer.h"
#include "CartridgeViewer.h"
#include "VramViewer.h"

#include <wx/wx.h>
#include <wx/notebook.h>
class Debugger : public wxFrame
{

public:
    Debugger(wxFrame *, Gameboy *, Memory *, Cartridge *);
    ~Debugger();

    void Update(Gameboy *, Memory *, Cartridge *);

private:
    Gameboy *gameboyRef;

    void OnClose(wxCloseEvent &);

    wxDECLARE_EVENT_TABLE();
};

#endif