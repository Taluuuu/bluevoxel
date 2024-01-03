#include "ui/ui_module.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "rendering/rendering_module.h"
#include "nuklear/ui_renderer_nuklear.h"
#include "windowing/windowing_module.h"

namespace h2o
{
    UIModule::UIModule()
        : Tickable(g_engine)
    {}

    void UIModule::window(
        const std::string& title,
        const ui::Rect& rect,
        const std::function<void(IUIRenderer&)>& window_contents)
    {
        assert(m_ui_renderer);

        if (m_ui_renderer->window_begin(title, rect))
            window_contents(*m_ui_renderer);

        m_ui_renderer->window_end();
    }

    bool UIModule::init(Engine& engine)
    {
        m_ui_renderer = std::make_shared<UIRenderer_Nuklear>(this);
        return m_ui_renderer->init(engine);
    }

    void UIModule::cleanup()
    {
        m_ui_renderer->cleanup();
        m_ui_renderer.reset();
    }

    std::vector<std::type_index> UIModule::dependencies() const
    {
        return {
            typeid(RenderingModule),
            typeid(WindowingModule),
            typeid(InputModule) };
    }
}