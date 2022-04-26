#include "App.hh"

static lr::BaseApp *pApp = nullptr;

lr::BaseApp *lr::GetApp()
{
    return pApp;
}

using namespace lr;

int main()
{
    ApplicationDesc desc;
    desc.Title = "Atmosphere";
    desc.Width = 1480;
    desc.Height = 820;
    desc.Flags |= WindowFlags::Resizable | WindowFlags::Centered;

    pApp = new AtmosphereApp;

    pApp->InitApp(desc);
    pApp->Run();

    return 0;
}