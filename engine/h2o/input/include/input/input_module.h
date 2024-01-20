#pragma once

#include "core/priority_value.h"
#include "core/core_enums.h"
#include "core/core_interfaces.h"
#include "core/module.h"
#include "core/events.h"
#include "key_state.h"

#include <variant>

namespace h2o
{
    class WindowingModule;

    enum class MouseCapturePriority : u32
    {
        Camera = 0,
        Game = 1,
        Editor = 2,
        UI = 3,
    };

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
        void register_axis(const std::string_view& name, MouseMoveDelta mouse_delta, f32 sensitivity = 1.0f, bool invert = false);
        void register_axis(const std::string_view& name, MouseScrollDelta scroll_delta, f32 sensitivity = 1.0f, bool invert = false);
        [[nodiscard]] f32 get_axis(const std::string_view& name);

        void set_mouse_state(MouseCapturePriority priority, bool captured);
        void clear_mouse_state(MouseCapturePriority priority);
        [[nodiscard]] bool is_mouse_captured() const;

        [[nodiscard]] KeyState key_state(Key key) const;
        [[nodiscard]] KeyState mouse_button_state(MouseButton button) const;
        [[nodiscard]] v2 mouse_position() const;
        [[nodiscard]] v2 mouse_delta() const;
        [[nodiscard]] v2 scroll_delta() const;

    private:

        // Input state
        std::vector<KeyState> m_key_states;
        std::vector<KeyState> m_mouse_button_states;
        v2 m_mouse_pos{}, m_mouse_delta{}, m_scroll_delta{};
        PriorityValue<MouseCapturePriority, bool> m_mouse_capture_state{};
        i32 m_mouse_move_frames_to_ignore = 0;

        // Input configuration
        struct KeyAxis { Key positive, negative; };
        struct MouseDeltaAxis { MouseMoveDelta delta; f32 sensitivity; bool invert; };
        struct MouseScrollAxis { MouseScrollDelta delta; f32 sensitivity; bool invert; };

        using InputMapping = std::variant<KeyAxis, MouseDeltaAxis, MouseScrollAxis>;
        std::unordered_map<std::string_view, InputMapping> m_input_axes;

        WindowingModule* m_windowing_module = nullptr;

        // Event handles
        EventHandle
            m_key_state_event_handle{},
            m_mouse_button_state_event_handle{},
            m_mouse_moved_event_handle{},
            m_mouse_scroll_event_handle{};

    };
}