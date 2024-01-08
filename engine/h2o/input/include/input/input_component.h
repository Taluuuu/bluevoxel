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

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;

        [[nodiscard]] f32 get_axis(const std::string& name) const;

        /**
         * Capture or free the mouse
         *
         * @param capture true if the mouse should be captured
         * @param allow_mouse_over_ui true if the mouse should be captured when it is hovering over UI
         */
        void set_capture_mouse(bool capture, bool allow_mouse_over_ui = true);
        [[nodiscard]] bool is_mouse_captured() const;

    private:

        InputModule* const m_input_module = nullptr;

    };
}