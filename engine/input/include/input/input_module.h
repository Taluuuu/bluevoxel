#pragma once

#include "core/module.h"
#include "windowing/windowing_module.h"
#include "input_enums.h"

namespace h2o
{
    struct KeyState
    {
        u8 held               : 1 = false;
        u8 pressed_this_frame : 1 = false;
    };

    class InputModule
        : public Module
        , public ITickable
    {
    public:
        
        explicit InputModule(Engine& engine);
        ~InputModule() override = default;

        // Module interface
        [[nodiscard]] std::string_view module_name() const override { return "h2o_input"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;

    private:

        std::vector<KeyState> m_key_states;

        WindowingModule* const m_windowing_module = nullptr;

    };
}