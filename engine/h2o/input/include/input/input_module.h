#pragma once

#include "core/core_enums.h"
#include "core/core_interfaces.h"
#include "core/module.h"
#include "core/events.h"
#include "key_state.h"

namespace h2o
{
    class InputModule
        : public IModule
        , public IInputModule
    {
    public:
        
        InputModule() = default;
        ~InputModule() override = default;

        // Module interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_input"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // IInputModule interface
        void prepare() override;

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;

    private:

        std::vector<KeyState> m_key_states;
        std::vector<KeyState> m_mouse_button_states;

        EventHandle m_key_state_event_handle;

    };
}