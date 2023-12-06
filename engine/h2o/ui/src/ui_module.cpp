#include "ui/ui_module.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "ui_renderer_nuklear.h"
#include "windowing/windowing_module.h"

namespace h2o
{
    UIModule::UIModule()
        : Tickable(g_engine)
    {}

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
        return { typeid(RenderingModule), typeid(WindowingModule) };
    }
}