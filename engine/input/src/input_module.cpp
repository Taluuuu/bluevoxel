#include "input/input_module.h"

#include "core/engine.h"
#include "windowing/windowing_module.h"
#include "windowing/window.h"

#include <magic_enum.hpp>

namespace h2o
{
    bool InputModule::init(Engine& engine)
    {
        m_key_states.resize(magic_enum::enum_count<Key>(), {});
        m_mouse_button_states.resize(magic_enum::enum_count<Key>(), {});

        m_windowing_module = engine.get_module<WindowingModule>();
        assert(m_windowing_module);

        m_key_state_event_handle = m_windowing_module->window().key_changed_event().add_listener([this]
            (KeyChangedEvent evt)
        {
            auto key_idx = magic_enum::enum_index(evt.key);
            assert(key_idx.has_value());

            m_key_states[*key_idx].held               = evt.pressed;
            m_key_states[*key_idx].pressed_this_frame = evt.pressed;
        });

        return true;
    }

    void InputModule::cleanup()
    {
        m_windowing_module->window().key_changed_event().remove_listener(m_key_state_event_handle);
    }

    std::vector<std::type_index> InputModule::dependencies() const
    {
        return { typeid(WindowingModule) };
    }

    void InputModule::tick(TickPhase phase, f64 delta_time)
    {
        for (auto& key_state : m_key_states)
            key_state.pressed_this_frame = false;
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
}