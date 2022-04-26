//
// Created on Saturday 23rd April 2022 by e-erdal
//

#pragma once

#include "Window.hh"
#include "InputManager.hh"

#include "Graphics/D3D11/D3D11API.hh"
#include "Graphics/Camera3D.hh"

#include "UI/ImGuiHandler.hh"

namespace lr
{
    struct ApplicationDesc
    {
        eastl::string Title = "Lorr";
        eastl::string Icon = "";

        WindowFlags Flags = WindowFlags::None;

        u32 Width = 0;
        u32 Height = 0;

        bool ConsoleApp = false;
    };

    class BaseApp
    {
    public:
        bool InitApp(ApplicationDesc const &desc);
        virtual void Init() = 0;

        void Run();

        // Graphics
        void BeginFrame();
        void EndFrame();

        // Logic
        virtual void Draw() = 0;
        virtual void Tick(float deltaTime) = 0;

    public:
        // clang-format off
        Window *GetWindow()         { return &m_Window; }
        D3D11API *GetAPI()          { return &m_API; }
        InputManager *GetInputMan() { return &m_InputMan; }
        Camera3D *GetCamera()       { return &m_Camera; }
        bool Initialized()          { return m_Initialized; }
        // clang-format on

    protected:
        Window m_Window;
        D3D11API m_API;
        InputManager m_InputMan;
        ImGuiHandler m_ImGui;

        Camera3D m_Camera;

        bool m_Initialized = false;
    };

    extern BaseApp *GetApp();

}  // namespace lr