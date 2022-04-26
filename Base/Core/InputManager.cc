#include "InputManager.hh"

#include "Core/BaseApp.hh"
#include "UI/ImGuiHandler.hh"

namespace lr
{
    static eastl::unordered_map<Key, Direction> kCameraMoveLUT = { { Key::Key_W, Direction::FORWARD }, { Key::Key_S, Direction::BACKWARD },
                                                                   { Key::Key_A, Direction::LEFT },    { Key::Key_D, Direction::RIGHT },
                                                                   { Key::Key_Q, Direction::UP },      { Key::Key_E, Direction::DOWN } };

    void InputManager::OnKeyboardState(ButtonState state, Key key, KeyMod mods)
    {
        if (ImGuiHandler::KeyPress(state, key, mods)) return;

        bool stop = state == ButtonState::Released;
        auto keyIt = kCameraMoveLUT.find(key);
        if (keyIt != kCameraMoveLUT.end())
        {
            GetApp()->GetCamera()->Move(keyIt->second, stop);
        }
    }

    void InputManager::OnMouseState(ButtonState state, MouseButton button, KeyMod mods)
    {
        m_MouseButtonState[button] = state;

        ImGuiHandler::MouseState(state, button, mods, m_MousePos);
    }

    void InputManager::OnMousePosUpdate(const XMINT2 &pos, const XMINT2 &offset)
    {
        m_MousePos = pos;

        if (ImGuiHandler::MousePosition(pos)) return;

        Camera3D *pCamera = GetApp()->GetCamera();

        bool dragAvailable = false;

        auto buttonIt = m_MouseButtonState.find(MouseButton::BTN_1);
        if (buttonIt == m_MouseButtonState.end())
            return;
        else if (buttonIt->second == ButtonState::Pressed)
            dragAvailable = true;

        if (dragAvailable)
        {
            pCamera->SetDirection(offset.x, offset.y);
        }
    }

    void InputManager::OnMouseWheelUpdate(float x, float y)
    {
        ImGuiHandler::MouseWheelState(x, y);
    }

    void InputManager::OnKeyInput(u32 utfChar)
    {
        ImGuiHandler::InputChar(utfChar);
    }

    void InputManager::SetMousePos(const XMINT2 &pos)
    {
        m_MousePos = pos;
    }

}  // namespace lr