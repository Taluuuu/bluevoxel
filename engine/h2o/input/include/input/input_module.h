#pragma once

#include "core/core_enums.h"
#include "core/core_interfaces.h"
#include "core/module.h"
#include "core/events.h"
#include "key_state.h"

#include <variant>

namespace h2o
{
    class InputModule
        : public IModule
        , public IInputModule // TODO: Remove this and make the module tick by itself
    {
    public:
        
        InputModule() = default;
        ~InputModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_input"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // IInputModule interface
        void prepare() override;

        void register_axis(const std::string_view& name, Key negative, Key positive);
        void register_axis(const std::string_view& name, MouseDelta mouse_delta, f32 sensitivity = 1.0f, bool invert = false);
        [[nodiscard]] f32 get_axis(const std::string_view& name);

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;
        [[nodiscard]] v2 mouse_delta() const;

    private:

        // Input state
        std::vector<KeyState> m_key_states;
        std::vector<KeyState> m_mouse_button_states;
        v2 m_mouse_pos{}, m_mouse_delta{};
        bool m_mouse_captured = false;
        i32 m_mouse_move_frames_to_ignore = false;

        // Input configuration
        struct KeyAxis { Key positive, negative; };
        struct MouseDeltaAxis { MouseDelta delta; f32 sensitivity; bool invert; };

        using InputMapping = std::variant<KeyAxis, MouseDeltaAxis>;
        std::unordered_map<std::string_view, InputMapping> m_input_axes;

        // Event handles
        EventHandle
            m_key_state_event_handle,
            m_mouse_button_state_event_handle,
            m_mouse_moved_event_handle;

    };
}