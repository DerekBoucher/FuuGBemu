#ifndef VRAMVIEWER_H
#define VRAMVIEWER_H

#include <wx/wx.h>
#include "Memory.h"

class VramViewer : public wxWindow
{

public:
    VramViewer(wxWindow *, Memory *);
    ~VramViewer();

private:
};

#endif