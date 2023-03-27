#pragma once

#include "core/core_enums.h"
#include "scene/component.h"
#include "key_state.h"

namespace h2o
{
    class InputModule;

    class InputComponent : public Component
    {
    public:

        explicit InputComponent(const ComponentInitializer& component_initializer);

        // TODO: Associate keys with names to allow remapping
        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;

    private:

        InputModule* const m_input_module = nullptr;

    };
}