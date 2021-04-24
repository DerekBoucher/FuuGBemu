#ifndef MEMORYVIEWER_H
#define MEMORYVIEWER_H

#include "Memory.h"
#include "CPU.h"

#include <wx/wx.h>
#include <wx/vscroll.h>
#include <wx/grid.h>
#include <map>

class MemoryViewer : public wxWindow
{

public:
    MemoryViewer(wxWindow *, Memory *);
    ~MemoryViewer();

private:
};

#endif