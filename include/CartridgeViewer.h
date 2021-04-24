#ifndef CARTRIDGEVIEWER_H
#define CARTRIDGEVIEWER_H

#include "Cartridge.h"

#include <wx/wx.h>

class CartridgeViewer : public wxWindow
{

public:
    CartridgeViewer(wxWindow *, Cartridge *);
    ~CartridgeViewer();

private:
    Cartridge *cartRef;
};

#endif