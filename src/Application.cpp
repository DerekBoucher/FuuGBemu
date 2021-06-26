#include "Application.h"

wxBEGIN_EVENT_TABLE(Application, wxApp)
    EVT_KEY_DOWN(Application::OnKeyDown)
        wxEND_EVENT_TABLE();

Application::Application()
{
}

Application::~Application()
{
}

bool Application::OnInit()
{
    gui = new GUI();
    gui->Show(true);

    return true;
}

void Application::OnKeyDown(wxKeyEvent &event)
{
    gui->OnKeyDown(event);
}