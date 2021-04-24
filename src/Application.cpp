#include "Application.h"

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