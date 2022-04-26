//
// Created on Monday 6th December 2021 by e-erdal
//

#pragma once

#include "InputVars.hh"

namespace lr
{
    class InputManager
    {
    public:
        InputManager() = default;

        void OnKeyboardState(ButtonState state, Key key, KeyMod mods);
        void OnMouseState(ButtonState state, MouseButton button, KeyMod mods);
        void OnMousePosUpdate(const XMINT2 &pos, const XMINT2 &offset);
        void OnMouseWheelUpdate(float x, float y);

        /// This should be used when keyboard is active
        void OnKeyInput(u32 utfChar);

    public:
        void SetMousePos(const XMINT2 &pos);
        const XMINT2 &GetMousePos() const
        {
            return m_MousePos;
        }

    private:
        XMINT2 m_MousePos;
        eastl::unordered_map<MouseButton, ButtonState> m_MouseButtonState;
    };

}  // namespace lr