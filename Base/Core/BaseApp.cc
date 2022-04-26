#include "BaseApp.hh"

#include "IO/BufferStream.hh"

namespace lr
{
    BufferStreamMemoyWatcher *g_pBSWatcher;

    bool BaseApp::InitApp(ApplicationDesc const &desc)
    {
        Logger::Init();
        LOG_TRACE("Initializing Lorr...");

        g_pBSWatcher = new BufferStreamMemoyWatcher(false);

        //* Core features
        m_Window.Init(desc.Title, 0, desc.Width, desc.Height, desc.Flags);

        //* Graphics
        m_API.Init(GetWindow(), m_Window.GetWidth(), m_Window.GetHeight(), APIFlags::None);
        m_Camera.Init(XMFLOAT3(0, 5, -5), XMFLOAT2(desc.Width, desc.Height), XMFLOAT3(0, 0, 1), XMFLOAT3(0, 1, 0), 60.f, 0.1f, 10000.f);
        m_ImGui.Init();

        Init();

        m_Initialized = true;

        return true;
    }

    void BaseApp::Run()
    {
        Timer timer;

        while (!m_Window.ShouldClose())
        {
            auto elapsed = timer.elapsed();
            timer.reset();

            m_Camera.Update(elapsed);
            Tick(elapsed);

            BeginFrame();
            Draw();
            EndFrame();

            m_Window.Poll();
        }
    }

    void BaseApp::BeginFrame()
    {
        m_API.BeginFrame();
        m_ImGui.BeginFrame();
    }

    void BaseApp::EndFrame()
    {
        m_ImGui.EndFrame();
        m_API.Frame();
    }

}  // namespace lr