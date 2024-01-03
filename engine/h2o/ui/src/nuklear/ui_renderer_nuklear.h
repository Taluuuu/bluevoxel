#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "nuklear_glfw_gl3.h"
#include "rendering/vertex_array.h"
#include "ui/ui_renderer.h"

#include <memory>

namespace h2o
{
    class InputModule;
    class RenderingModule;
    class WindowingModule;

    namespace gfx
    {
        class Buffer;
        class IPipeline;
        class IRenderer;
        class Texture;
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
        bool window_begin(const std::string& title, const ui::Rect& rect) override;
        void window_end() override;
        void row(f32 height, i32 num_columns) override;
        void label(const std::string& label) override;
        bool button(const std::string& title) override;
        bool input_text(const std::string& label, std::string& text) override;
        bool input_int(const std::string& label, i32& num) override;

        // Tickable interface
        void frame_start(f32 delta_time) override;
        void frame_end(f32 delta_time) override;

    private:

        struct NkVertex
        {
            v2 position{};
            v2 uv{};
            u8 col[4];
        };

        nk_context m_nk_ctx{};
        nk_buffer m_nk_commands{};
        nk_draw_null_texture m_nk_texture_null{};
        nk_font_atlas m_nk_atlas{};
        std::shared_ptr<gfx::Texture> m_font_texture{};

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

        std::vector<u32> m_pressed_unicode_chars{};
        EventHandle m_char_event_handle{};

        InputModule*     m_input_module     = nullptr;
        RenderingModule* m_rendering_module = nullptr;
        WindowingModule* m_windowing_module = nullptr;

    };
}