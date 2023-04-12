#include "input/input_module.h"

#include "core/engine.h"
#include "core/log.h"
#include "scene/scene_module.h"
#include "windowing/windowing_module.h"
#include "windowing/window.h"

#include <magic_enum.hpp>

namespace h2o
{
    bool InputModule::init(Engine& engine)
    {
        m_key_states.resize(magic_enum::enum_count<Key>(), {});
        m_mouse_button_states.resize(magic_enum::enum_count<Key>(), {});

        auto windowing_module = engine.get_module<WindowingModule>();
        assert(windowing_module);

        windowing_module->window().key_changed_event().add_listener(m_key_state_event_handle,
            [&, windowing_module](const KeyChangedEvent& evt)
            {
                auto key_idx = magic_enum::enum_index(evt.key);
                assert(key_idx.has_value());

                m_key_states[*key_idx].held               = evt.pressed;
                m_key_states[*key_idx].pressed_this_frame = evt.pressed;

                // Hard-coded capture mouse key
                if (evt.key == Key::Escape)
                {
                    if (evt.pressed)
                    {
                        m_mouse_captured = !m_mouse_captured;
                        windowing_module->window().set_capture_mouse(m_mouse_captured);
                    }

                    // Hack, see comment below
                    m_mouse_move_frames_to_ignore = 2;
                }
            });

        windowing_module->window().mouse_moved_event().add_listener(m_mouse_moved_event_handle,
            [&](const MouseMovedEvent& evt)
            {
                if (m_mouse_captured)
                    m_mouse_delta = evt.new_position - m_mouse_pos;

                // Hack to fix mouse jumping when spamming capture/release mouse
                // while moving the mouse. Somehow sometimes a high delta is
                // calculated by GLFW over two frames. This fixes it.
                if (m_mouse_move_frames_to_ignore > 0)
                {
                    m_mouse_delta = {};
                    m_mouse_move_frames_to_ignore--;
                }

                m_mouse_pos = evt.new_position;
            });

        return true;
    }

    std::vector<std::type_index> InputModule::dependencies() const
    {
        return { typeid(WindowingModule), typeid(SceneModule) };
    }

    void InputModule::prepare()
    {
        for (auto& key_state : m_key_states)
            key_state.pressed_this_frame = false;

        m_mouse_delta = { 0.0f, 0.0f };
    }

    void InputModule::register_axis(const std::string_view& name, Key negative, Key positive)
    {
        const auto it = m_input_axes.find(name);
        if (it != m_input_axes.end())
        {
            log::warn("Trying to register two axes with the same name: '{}'.", name);
            return;
        }

        m_input_axes.insert({name, KeyAxis { positive, negative } });
    }

    void InputModule::register_axis(const std::string_view& name, MouseDelta mouse_delta, f32 sensitivity, bool invert)
    {
        const auto it = m_input_axes.find(name);
        if (it != m_input_axes.end())
        {
            log::warn("Trying to register two axes with the same name: '{}'.", name);
            return;
        }

        m_input_axes.insert(
            { name, MouseDeltaAxis { mouse_delta, sensitivity, invert } });
    }

    f32 InputModule::get_axis(const std::string_view& name)
    {
        const auto it = m_input_axes.find(name);
        if (it == m_input_axes.end())
        {
            log::warn("Trying to get unregistered axis with name: '{}'.", name);
            return 0.0f;
        }

        const auto& axis = it->second;

        if (auto key_axis = get_if<KeyAxis>(&axis))
        {
            return
                static_cast<f32>(key_state(key_axis->positive).held) -
                static_cast<f32>(key_state(key_axis->negative).held);
        }

        if (auto mouse_axis = get_if<MouseDeltaAxis>(&axis))
        {
            float val = 0.0f;
            switch (mouse_axis->delta)
            {
            case MouseDelta::X: val = mouse_delta().x; break;
            case MouseDelta::Y: val = mouse_delta().y; break;
            }

            return val * (mouse_axis->invert ? -1.0f : 1.0f) * mouse_axis->sensitivity;
        }

        return 0.0f;
    }

    KeyState InputModule::key_state(Key key) const
    {
        auto key_idx = magic_enum::enum_index(key);
        assert(key_idx.has_value());

        return m_key_states[*key_idx];
    }

    KeyState InputModule::mouse_button_state(MouseButton button) const
    {
        auto btn_idx = magic_enum::enum_index(button);
        assert(btn_idx.has_value());

        return m_key_states[*btn_idx];
    }

    v2 InputModule::mouse_delta() const
    {
        return m_mouse_delta;
    }
}