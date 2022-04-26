#include "ImGuiHandler.hh"

#include "Core/BaseApp.hh"

namespace lr
{
    static InputLayout kImGuiInputLayout = {
        { VertexAttribType::Vec2, "POSITION" },
        { VertexAttribType::Vec2, "TEXCOORD" },
        { VertexAttribType::UInt4PKN, "COLOR" },
    };

    static u32 g_VertexBufferSize = 5000;
    static u32 g_IndexBufferSize = 10000;

    bool ImGuiHandler::KeyPress(ButtonState state, Key key, KeyMod mods)
    {
        if (ImGui::GetCurrentContext() == NULL) return false;

        ImGuiIO &io = ImGui::GetIO();

        io.KeyCtrl = mods & KeyMod::CONTROL;
        io.KeyShift = mods & KeyMod::SHIFT;
        io.KeyAlt = mods & KeyMod::ALT;
        io.KeySuper = mods & KeyMod::SUPER;

        if ((int)key < IM_ARRAYSIZE(io.KeysDown))
        {
            io.KeysDown[(int)key] = state == ButtonState::Pressed;
        }

        return io.WantCaptureKeyboard;
    }

    void ImGuiHandler::InputChar(u32 character)
    {
        if (ImGui::GetCurrentContext() == NULL) return;

        ImGuiIO &io = ImGui::GetIO();

        io.AddInputCharacterUTF16(character);
    }

    bool ImGuiHandler::MouseState(ButtonState state, MouseButton button, KeyMod mods, const XMINT2 &pos)
    {
        if (ImGui::GetCurrentContext() == NULL) return false;

        ImGuiIO &io = ImGui::GetIO();

        int mouse = 0;

        switch (button)
        {
            case MouseButton::BTN_1: mouse = 0; break;
            case MouseButton::BTN_2: mouse = 1; break;
            case MouseButton::BTN_3: mouse = 2; break;
            case MouseButton::BTN_4: mouse = 3; break;
            case MouseButton::BTN_5: mouse = 4; break;
            default: break;
        }

        io.AddMouseButtonEvent(mouse, state == ButtonState::Pressed || state == ButtonState::DoubleClicked);

        return io.WantCaptureMouse;
    }

    bool ImGuiHandler::MouseWheelState(float x, float y)
    {
        if (ImGui::GetCurrentContext() == NULL) return false;

        ImGuiIO &io = ImGui::GetIO();
        io.AddMouseWheelEvent(x, y);

        return io.WantCaptureMouse;
    }

    void ImGuiHandler::UpdateMouse()
    {
        if (ImGui::GetCurrentContext() == NULL) return;

        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) return;

        ImGuiViewport *main_viewport = ImGui::GetMainViewport();
        Window *pSurface = (Window *)io.BackendPlatformUserData;

        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
        {
            // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
            pSurface->SetCursor(Cursor::Hidden);
        }
        else
        {
            // Show OS mouse cursor
            auto cursor = Cursor::Arrow;
            switch (imgui_cursor)
            {
                case ImGuiMouseCursor_Arrow: cursor = Cursor::Arrow; break;
                case ImGuiMouseCursor_TextInput: cursor = Cursor::TextInput; break;
                case ImGuiMouseCursor_ResizeAll: cursor = Cursor::ResizeAll; break;
                case ImGuiMouseCursor_ResizeEW: cursor = Cursor::ResizeEW; break;
                case ImGuiMouseCursor_ResizeNS: cursor = Cursor::ResizeNS; break;
                case ImGuiMouseCursor_ResizeNESW: cursor = Cursor::ResizeNESW; break;
                case ImGuiMouseCursor_ResizeNWSE: cursor = Cursor::ResizeNWSE; break;
                case ImGuiMouseCursor_Hand: cursor = Cursor::Hand; break;
                case ImGuiMouseCursor_NotAllowed: cursor = Cursor::NotAllowed; break;
            }

            pSurface->SetCursor(cursor);
        }
    }

    void RefreshFonts()
    {
        auto &io = ImGui::GetIO();

        io.Fonts->Build();

        u8 *pFontData;
        i32 fontW, fontH;
        io.Fonts->GetTexDataAsRGBA32(&pFontData, &fontW, &fontH);

        TextureDesc desc;
        TextureData texData{ .Width = (u32)fontW, .Height = (u32)fontH, .Format = TextureFormat::RGBA8, .Data = pFontData };

        Texture *pFont = new Texture;
        pFont->Init(&desc, &texData);

        io.Fonts->TexID = pFont;  // We dont use IDs
    }

    bool ImGuiHandler::MousePosition(const XMINT2 &pos)
    {
        if (ImGui::GetCurrentContext() == NULL) return false;

        ImGuiIO &io = ImGui::GetIO();

        InputManager *pInputMan = GetApp()->GetInputMan();

        if (io.WantSetMousePos)
        {
            pInputMan->SetMousePos(XMINT2(io.MousePos.x, io.MousePos.y));
        }

        io.AddMousePosEvent(pos.x, pos.y);
        io.MouseHoveredViewport = 0;
        return io.WantCaptureMouse;
    }

    ImGuiHandler::~ImGuiHandler()
    {
    }

    void ImGuiHandler::Init()
    {
        ImGui::CreateContext();

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
        io.IniFilename = 0;

        ImGui::StyleColorsDark();

        InitImGui(GetApp()->GetWindow());
    }

    void ImGuiHandler::BeginFrame()
    {
        ImGui_ImplSurface_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiHandler::EndFrame()
    {
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplIRenderer_Draw();
    }

    void ImGuiHandler::InitImGui(Window *pWindow)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;          // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;           // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;  // We can set io.MouseHoveredViewport correctly (optional, not easy)
        // io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;  // We can create multi-viewports on the Platform side (optional)
        io.BackendPlatformName = "imgui_impl_surface";

        io.KeyMap[ImGuiKey_Tab] = (int)Key::Key_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = (int)Key::Key_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = (int)Key::Key_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = (int)Key::Key_UP;
        io.KeyMap[ImGuiKey_DownArrow] = (int)Key::Key_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = (int)Key::Key_PAGE_UP;
        io.KeyMap[ImGuiKey_PageDown] = (int)Key::Key_PAGE_DOWN;
        io.KeyMap[ImGuiKey_Home] = (int)Key::Key_HOME;
        io.KeyMap[ImGuiKey_End] = (int)Key::Key_END;
        io.KeyMap[ImGuiKey_Insert] = (int)Key::Key_INSERT;
        io.KeyMap[ImGuiKey_Delete] = (int)Key::Key_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = (int)Key::Key_BACKSPACE;
        io.KeyMap[ImGuiKey_Space] = (int)Key::Key_SPACE;
        io.KeyMap[ImGuiKey_Enter] = (int)Key::Key_ENTER;
        io.KeyMap[ImGuiKey_Escape] = (int)Key::Key_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = (int)Key::Key_KP_ENTER;
        io.KeyMap[ImGuiKey_A] = (int)Key::Key_A;
        io.KeyMap[ImGuiKey_C] = (int)Key::Key_C;
        io.KeyMap[ImGuiKey_V] = (int)Key::Key_V;
        io.KeyMap[ImGuiKey_X] = (int)Key::Key_X;
        io.KeyMap[ImGuiKey_Y] = (int)Key::Key_Y;
        io.KeyMap[ImGuiKey_Z] = (int)Key::Key_Z;

        io.BackendPlatformUserData = (void *)pWindow;

        ImGuiViewport *main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformHandle = (void *)pWindow->GetHandle();

        //* IRenderer backend initialization *//
        ShaderDesc shaderDesc;
        shaderDesc.pLayout = &kImGuiInputLayout;
        shaderDesc.Type = ShaderType::Vertex;

        m_VertexShader.Init(&shaderDesc, "Common/imgui");

        shaderDesc.pLayout = nullptr;
        shaderDesc.Type = ShaderType::Pixel;
        m_PixelShader.Init(&shaderDesc, "Common/imgui");

        RefreshFonts();

        RenderBufferDesc cbDesc;
        cbDesc.DataLen = sizeof(float[4][4]);
        cbDesc.Type = RenderBufferType::Constant;
        cbDesc.Usage = RenderBufferUsage::Dynamic;
        cbDesc.MemFlags = RenderBufferMemoryFlags::Access_CPUW;

        m_VertexConstantBuffer.Init(cbDesc);

        cbDesc.DataLen = sizeof(XMMATRIX);
        m_PixelConstantBuffer.Init(cbDesc);
    }

    void ImGuiHandler::ImGui_ImplSurface_Shutdown()
    {
    }

    void ImGuiHandler::ImGui_ImplSurface_NewFrame()
    {
        static Cursor lastCursor = Cursor::Hidden;
        static Timer timer{};

        ImGuiIO &io = ImGui::GetIO();
        IM_ASSERT(io.Fonts->IsBuilt()
                  && "Font atlas not built! It is generally built by the renderer backend. Missing "
                     "call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

        // Setup display size (every frame to accommodate for window resizing)
        ImGuiViewport *main_viewport = ImGui::GetMainViewport();
        Window *pSurface = (Window *)io.BackendPlatformUserData;

        io.DisplaySize = ImVec2((float)(pSurface->GetWidth()), (float)(pSurface->GetHeight()));

        // Setup time step
        io.DeltaTime = timer.elapsed();

        // Update OS mouse cursor with the cursor requested by imgui
        ImGuiMouseCursor mouse_cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
        if (lastCursor != (Cursor)mouse_cursor)
        {
            lastCursor = (Cursor)mouse_cursor;
            UpdateMouse();
        }

        timer.reset();
    }

    void ImGuiHandler::ImGui_ImplIRenderer_Draw()
    {
        ImDrawData *pDrawData = ImGui::GetDrawData();
        D3D11API *pRenderer = GetApp()->GetAPI();
        Window *pWindow = GetApp()->GetWindow();

        pRenderer->SetRenderTarget(nullptr);

        pRenderer->SetDepthFunc(DepthFunc::Always, false);
        pRenderer->SetCulling(Cull::None, false);
        pRenderer->SetBlend(true, false);
        pRenderer->SetPrimitiveType(PrimitiveType::TriangleList);

        if (pDrawData->DisplaySize.x <= 0.0f || pDrawData->DisplaySize.y <= 0.0f) return;

        if (!m_VertexBuffer.Valid() || g_VertexBufferSize < pDrawData->TotalVtxCount)
        {
            m_VertexBuffer.Delete();
            g_VertexBufferSize = pDrawData->TotalVtxCount + 5000;

            RenderBufferDesc desc;
            desc.DataLen = g_VertexBufferSize * sizeof(ImDrawVert);
            desc.Type = RenderBufferType::Vertex;
            desc.Usage = RenderBufferUsage::Dynamic;
            desc.MemFlags = RenderBufferMemoryFlags::Access_CPUW;

            m_VertexBuffer.Init(desc);
        }

        if (!m_IndexBuffer.Valid() || g_IndexBufferSize < pDrawData->TotalIdxCount)
        {
            m_IndexBuffer.Delete();
            g_IndexBufferSize = pDrawData->TotalIdxCount + 10000;

            RenderBufferDesc desc;
            desc.DataLen = g_IndexBufferSize * sizeof(ImDrawIdx);
            desc.Type = RenderBufferType::Index;
            desc.Usage = RenderBufferUsage::Dynamic;
            desc.MemFlags = RenderBufferMemoryFlags::Access_CPUW;

            m_IndexBuffer.Init(desc);
        }

        ImDrawVert *pVertexData = (ImDrawVert *)malloc(g_VertexBufferSize * sizeof(ImDrawVert));
        ImDrawVert *pVertex = pVertexData;
        ImDrawIdx *pIndexData = (ImDrawIdx *)malloc(g_IndexBufferSize * sizeof(ImDrawIdx));
        ImDrawIdx *pIndex = pIndexData;

        for (int i = 0; i < pDrawData->CmdListsCount; i++)
        {
            const ImDrawList *pDrawList = pDrawData->CmdLists[i];
            memcpy(pVertex, pDrawList->VtxBuffer.Data, pDrawList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(pIndex, pDrawList->IdxBuffer.Data, pDrawList->IdxBuffer.Size * sizeof(ImDrawIdx));

            pVertex += pDrawList->VtxBuffer.Size;
            pIndex += pDrawList->IdxBuffer.Size;
        }

        pRenderer->MapBuffer(&m_VertexBuffer, pVertexData, g_VertexBufferSize * sizeof(ImDrawVert));
        pRenderer->MapBuffer(&m_IndexBuffer, pIndexData, g_IndexBufferSize * sizeof(ImDrawIdx));

        free(pVertexData);
        free(pIndexData);

        float R = pWindow->GetWidth();
        float B = pWindow->GetHeight();

        // clang-format off
        XMMATRIX matrix = XMMatrixSet(
            2.0f / R, 0.0f, 0.0f, 0.0f, 
            0.0f, 2.0f / -B, 0.0f, 0.0f, 
            0.0f, 0.0f, 0.5f, 0.0f, 
            -1.0f, 1.0f, 0.5f, 1.0f);
        // clang-format on

        pRenderer->MapBuffer(&m_VertexConstantBuffer, &matrix, sizeof(XMMATRIX));

        pRenderer->SetShader(&m_VertexShader);
        pRenderer->SetShader(&m_PixelShader);

        pRenderer->SetConstantBuffer(&m_VertexConstantBuffer, RenderBufferTarget::Vertex, 0);
        pRenderer->SetVertexBuffer(&m_VertexBuffer, &kImGuiInputLayout);
        pRenderer->SetIndexBuffer(&m_IndexBuffer, false);

        u32 vertexOff = 0;
        u32 indexOff = 0;
        XMFLOAT2 clipOff = { pDrawData->DisplayPos.x, pDrawData->DisplayPos.y };

        for (int i = 0; i < pDrawData->CmdListsCount; i++)
        {
            const ImDrawList *pDrawList = pDrawData->CmdLists[i];

            for (auto &cmd : pDrawList->CmdBuffer)
            {
                if (cmd.UserCallback)
                {
                    if (cmd.UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        LOG_ERROR("Setup Render state!!!");
                    }
                    else
                    {
                        cmd.UserCallback(pDrawList, &cmd);
                    }
                }
                else
                {
                    XMUINT2 clipMin(cmd.ClipRect.x - clipOff.x, cmd.ClipRect.y - clipOff.y);
                    XMUINT2 clipMax(cmd.ClipRect.z - clipOff.x, cmd.ClipRect.w - clipOff.y);
                    if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y) continue;

                    pRenderer->SetScissor({ clipMin.x, clipMin.y, clipMax.x, clipMax.y });

                    Texture *pTexture = cmd.TextureId;
                    // Wow, this is just too big brain, now we avoid rendering null textures
                    // Thank you myself
                    if (pTexture)
                    {
                        XMVECTOR mips = { (float)pTexture->GetCurrentMip(), 0.f, 0.f, 0.f };

                        pRenderer->MapBuffer(&m_PixelConstantBuffer, &mips, sizeof(XMVECTOR));

                        pRenderer->SetConstantBuffer(&m_PixelConstantBuffer, RenderBufferTarget::Pixel, 0);

                        pRenderer->SetShaderResource(pTexture, RenderBufferTarget::Pixel, 0);
                        pRenderer->SetSamplerState(TextureFiltering::Linear,
                                                   TextureAddress::Mirror,
                                                   TextureAddress::Mirror,
                                                   TextureAddress::Mirror,
                                                   RenderBufferTarget::Pixel,
                                                   0);
                    }

                    pRenderer->DrawIndexed(cmd.ElemCount, cmd.IdxOffset + indexOff, cmd.VtxOffset + vertexOff);
                }
            }

            vertexOff += pDrawList->VtxBuffer.Size;
            indexOff += pDrawList->IdxBuffer.Size;
        }

        pRenderer->SetScissor({ 0, 0, pWindow->GetWidth(), pWindow->GetHeight() });
    }

}  // namespace lr