#pragma once

#include "core/tickable.h"
#include "nuklear_glfw_gl3.h"
#include "rendering/vertex_array.h"
#include "ui/ui_renderer.h"

#include <memory>

namespace h2o
{
    class RenderingModule;
    class WindowingModule;

    namespace gfx
    {
        class Buffer;
        class IPipeline;
        class IRenderer;
    }

    class UIRenderer_Nuklear
        : public Tickable
        , public IUIRenderer
    {
    public:

        explicit UIRenderer_Nuklear(Tickable* owner);
        ~UIRenderer_Nuklear() override = default;

        // IRendererUI interface
        bool init(Engine& engine) override;
        void cleanup() override;
        void window(const std::string& title, const ui::Rect& rect, const std::function<void()>& window_contents) override;
        void row(f32 height, i32 num_columns) override;
        bool button(const std::string& title) override;

        // Tickable interface
        void frame_start(f32 delta_time) override;
        void frame_end(f32 delta_time) override;

    private:

        struct NkVertex
        {
            v2 position{};
            v2 uv{};
            v4 col{};
        };

        nk_context m_nk_context{};
        nk_buffer m_nk_commands{};
        nk_draw_null_texture m_nk_texture_null{};

        std::shared_ptr<gfx::IPipeline> m_nuklear_pipeline = nullptr;

        static constexpr i32 max_vertex_buffer = 512 * 1024;
        std::shared_ptr<gfx::Buffer> m_nuklear_vertex_buffer = nullptr;
        static constexpr i32 max_index_buffer = 128 * 1024;
        std::shared_ptr<gfx::Buffer> m_nuklear_index_buffer = nullptr;
        std::shared_ptr<gfx::VertexArray> m_nuklear_vertex_array = nullptr;

        // Pipeline locations
        i32 m_uniform_tex = 0;
        i32 m_uniform_proj = 0;
        i32 m_attrib_pos = 0;
        i32 m_attrib_uv = 0;
        i32 m_attrib_col = 0;

        RenderingModule* m_rendering_module = nullptr;
        WindowingModule* m_windowing_module = nullptr;

    };
}