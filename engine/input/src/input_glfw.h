#pragma once

#include "input.h"

namespace h2o::input
{
    class Input_GLFW : public IInput
    {
    public:

        // IInput interface
        KeyState key_state(Key key) override;
        KeyState mouse_button_state(MouseButton button) override;

    };
}