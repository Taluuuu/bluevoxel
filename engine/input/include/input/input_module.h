#pragma once

#include "core/core_enums.h"
#include "core/core_interfaces.h"
#include "core/module.h"
#include "core/events.h"

namespace h2o
{
    struct KeyState
    {
        u8 held               : 1 = false;
        u8 pressed_this_frame : 1 = false;
    };

    class InputModule
        : public IModule
        , public ITickable
    {
    public:
        
        InputModule() = default;
        ~InputModule() override = default;

        // Module interface
        bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_input"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // ITickable interface
        void tick(TickPhase phase, f64 delta_time) override;

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;

    private:

        std::vector<KeyState> m_key_states;
        std::vector<KeyState> m_mouse_button_states;

        class WindowingModule* m_windowing_module = nullptr;

        EventHandle m_key_state_event_handle;

    };
}