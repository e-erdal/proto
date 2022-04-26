//
// Created on July 10th 2021 by e-erdal.
//

#pragma once

#include <imgui.h>

#include "Core/InputVars.hh"

#include "Graphics/D3D11/D3D11Shader.hh"
#include "Graphics/D3D11/D3D11RenderBuffer.hh"

namespace lr
{
    class Window;
    class ImGuiHandler
    {
    public:
        ~ImGuiHandler();

        void Init();

        void BeginFrame();
        void EndFrame();

        static bool KeyPress(ButtonState state, Key key, KeyMod mods);
        static void InputChar(u32 character);
        static bool MouseState(ButtonState state, MouseButton button, KeyMod mods, const XMINT2 &pos);
        static bool MouseWheelState(float x, float y);
        static bool MousePosition(const XMINT2 &pos);
        static void UpdateMouse();

    private:
        void InitImGui(Window *pWindow);
        void ImGui_ImplSurface_Shutdown();
        void ImGui_ImplSurface_NewFrame();
        void ImGui_ImplIRenderer_Draw();

    private:
        Shader m_VertexShader;
        Shader m_PixelShader;

        RenderBuffer m_VertexBuffer;
        RenderBuffer m_IndexBuffer;
        RenderBuffer m_VertexConstantBuffer;
        RenderBuffer m_PixelConstantBuffer;
    };
}  // namespace lr