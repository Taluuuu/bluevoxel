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
            [&](KeyChangedEvent evt)
            {
                auto key_idx = magic_enum::enum_index(evt.key);
                assert(key_idx.has_value());

                m_key_states[*key_idx].held               = evt.pressed;
                m_key_states[*key_idx].pressed_this_frame = evt.pressed;
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
    }

    void InputModule::register_axis(const std::string_view& name, Key negative, Key positive)
    {
        const auto it = m_input_axes.find(name);
        if (it != m_input_axes.end())
        {
            log::warn("Trying to register two axes with the same name: '{}'.", name);
            return;
        }

        m_input_axes.insert({ name, { positive, negative } });
    }

    f32 InputModule::get_axis(const std::string_view& name)
    {
        const auto it = m_input_axes.find(name);
        if (it == m_input_axes.end())
        {
            log::warn("Trying to get unregistered axis with name: '{}'.", name);
            return 0.0f;
        }

        // Currently only keyboard implementation
        const auto& axis = it->second;

        const f32 value =
            static_cast<f32>(key_state(axis.positive).held) -
            static_cast<f32>(key_state(axis.negative).held);

        return value;
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