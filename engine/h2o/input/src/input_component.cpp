#include "input/input_component.h"

#include "core/engine.h"
#include "input/input_module.h"

namespace h2o
{
    InputComponent::InputComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
        , m_input_module(g_engine->get_module<InputModule>())
    {
        assert(m_input_module);
    }

    KeyState InputComponent::key_state(Key key) const
    {
        return m_input_module ?
            m_input_module->key_state(key) : KeyState{};
    }

    KeyState InputComponent::mouse_button_state(MouseButton button) const
    {
        return m_input_module ?
            m_input_module->mouse_button_state(button) : KeyState{};
    }

    f32 InputComponent::get_axis(const std::string& name) const
    {
        return m_input_module ?
            m_input_module->get_axis(name) : 0.0f;
    }

    void InputComponent::set_capture_mouse(bool capture)
    {
        m_input_module->set_capture_mouse(capture);
    }

    bool InputComponent::is_mouse_captured() const
    {
        return m_input_module->is_mouse_captured();
    }
}