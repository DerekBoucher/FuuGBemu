#ifndef APPLICATION_H
#define APPLICATION_H

#include "Defines.h"
#include "GUI.h"

#include <wx/app.h>

class Application : public wxApp
{

public:
    Application();
    ~Application();

    wxDECLARE_EVENT_TABLE();
    virtual bool OnInit();
    void OnKeyDown(wxKeyEvent &);

private:
    GUI *gui;
};

wxIMPLEMENT_APP_NO_MAIN(Application);

#endif