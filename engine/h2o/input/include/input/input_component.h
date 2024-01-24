#pragma once

#include "core/core_enums.h"
#include "input_module.h" // TODO: Remove this include
#include "key_state.h"
#include "scene/component.h"

namespace h2o
{
    class InputModule;

    class InputComponent : public Component
    {
    public:

        explicit InputComponent(const ComponentInitializer& component_initializer);

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;

        [[nodiscard]] f32 get_axis(const std::string& name) const;

        void set_capture_mouse(MouseCapturePriority priority, bool capture);
        [[nodiscard]] bool is_mouse_captured() const;
        [[nodiscard]] std::optional<MouseCapturePriority> mouse_capture_priority() const;

    private:

        InputModule* const m_input_module = nullptr;

    };
}