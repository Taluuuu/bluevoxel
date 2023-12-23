#include "ui_renderer_nuklear.h"

#include "core/engine.h"
#include "nuklear_utils.h"
#include "rendering/buffer.h"
#include "rendering/pipeline.h"
#include "rendering/renderer_base.h"
#include "rendering/rendering_module.h"
#include "rendering/vertex_array.h"
#include "windowing/windowing_module.h"

namespace h2o
{
    UIRenderer_Nuklear::UIRenderer_Nuklear(Tickable* owner)
        : Tickable(owner)
    {}

    bool UIRenderer_Nuklear::init(Engine& engine)
    {
        m_rendering_module = &engine.get_module_checked<RenderingModule>();
        m_windowing_module = &engine.get_module_checked<WindowingModule>();

        const auto& rendering_module = engine.get_module_checked<RenderingModule>();
        gfx::IRenderer& renderer = rendering_module.renderer();

        nk_init_default(&m_nk_context, nullptr);
        nk_buffer_init_default(&m_nk_commands);

        // Setup Nuklear rendering pipeline
        m_nuklear_pipeline = renderer
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex, "../Resources/engine/shaders/opengl/nuklear_ui.vert")
            .add_shader(gfx::ShaderStage::Fragment, "../Resources/engine/shaders/opengl/nuklear_ui.frag")
            .with_feature(gfx::PipelineFeature::Blend)
            .with_blend_config(gfx::BlendEquation::Add, gfx::BlendFactor::SrcAlpha, gfx::BlendFactor::OneMinusSrcAlpha)
            .with_feature(gfx::PipelineFeature::ScissorTest)
            .compile();

        if (!m_nuklear_pipeline)
            return false;

        m_uniform_tex = m_nuklear_pipeline->get_uniform_location("Texture");
        m_uniform_proj = m_nuklear_pipeline->get_uniform_location("ProjMtx");
        m_attrib_pos = m_nuklear_pipeline->get_attribute_location("Position");
        m_attrib_uv = m_nuklear_pipeline->get_attribute_location("TexCoord");
        m_attrib_col = m_nuklear_pipeline->get_attribute_location("Color");

        // Create VAO and VBOs
        m_nuklear_vertex_array = renderer.create_vertex_array_ptr();
        m_nuklear_index_buffer = renderer.create_buffer_ptr();
        m_nuklear_index_buffer->update_data(nullptr, max_index_buffer, gfx::BufferUsage::StreamDraw);
        m_nuklear_vertex_buffer = renderer.create_buffer_ptr();
        m_nuklear_vertex_buffer->update_data(nullptr, max_vertex_buffer, gfx::BufferUsage::StreamDraw);

        m_nuklear_vertex_array->attach_index_buffer(m_nuklear_index_buffer);
        m_nuklear_vertex_array->attach_vertex_buffer(m_nuklear_vertex_buffer, 0, 0, sizeof(NkVertex));
        m_nuklear_vertex_array->setup_attribute(m_attrib_pos, 0, gfx::AttributeType::F32, 2, offsetof(NkVertex, position));
        m_nuklear_vertex_array->setup_attribute(m_attrib_uv,  0, gfx::AttributeType::F32, 2, offsetof(NkVertex, uv));
        m_nuklear_vertex_array->setup_attribute(m_attrib_col, 0, gfx::AttributeType::F32, 4, offsetof(NkVertex, col));

        set_tick_phases(TickPhase::FrameStart | TickPhase::FrameEnd);
        return true;
    }

    void UIRenderer_Nuklear::cleanup()
    {

    }

    void UIRenderer_Nuklear::window(
        const std::string& title,
        const ui::Rect& rect,
        const std::function<void()>& window_contents)
    {
//        nk_begin(m_nk_context, title.c_str(), ui::to_nk_rect(rect), 0);

        window_contents();

//        nk_end(m_nk_context);
    }

    void UIRenderer_Nuklear::row(f32 height, i32 num_columns)
    {
//        nk_layout_row_dynamic(m_nk_context, height, num_columns);
    }

    bool UIRenderer_Nuklear::button(const std::string& title)
    {
//        return nk_button_label(m_nk_context, title.c_str());
        return false;
    }

    void UIRenderer_Nuklear::frame_start(f32 delta_time)
    {
//        nk_glfw3_new_frame(&m_glfw);
    }

    void UIRenderer_Nuklear::frame_end(f32 delta_time)
    {
        const IWindow& window = m_windowing_module->window();

        const v2i window_size = window.window_size();
        const v2i framebuffer_size = window.framebuffer_size();
        const v2 framebuffer_scale = v2(framebuffer_size) / v2(window_size);

        const m4 ortho {
             2.0f / f32(window_size.x),  0.0f,                       0.0f,  0.0f,
             0.0f,                      -2.0f / f32(window_size.y),  0.0f,  0.0f,
             0.0f,                       0.0f,                      -1.0f,  0.0f,
            -1.0f,                       1.0f,                       0.0f,  1.0f
        };

        nk_buffer vbuf{}, ebuf{};

        m_nuklear_pipeline->set_uniform_int(m_uniform_tex, 0);
        m_nuklear_pipeline->set_uniform_mat4(m_uniform_proj, ortho);

        static const nk_draw_vertex_layout_element vertex_layout[] {
            { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NkVertex, position)},
            { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct NkVertex, uv)},
            { NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct NkVertex, col)},
            { NK_VERTEX_LAYOUT_END}
        };

        nk_convert_config config
        {
            .global_alpha = 0.0f,
            .line_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON,
            .shape_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON,
            .circle_segment_count = 22,
            .arc_segment_count = 22,
            .curve_segment_count = 22,
            .tex_null = m_nk_texture_null,
            .vertex_layout = vertex_layout,
            .vertex_size = sizeof(NkVertex),
            .vertex_alignment = NK_ALIGNOF(NkVertex),
        };

        m_nuklear_vertex_buffer->map_write_only(
            [&](void* vertex_data, size_t index_length)
            {
                m_nuklear_index_buffer->map_write_only(
                    [&](void* index_data, size_t vertex_length)
                    {
                        nk_buffer_init_fixed(&vbuf, vertex_data, max_vertex_buffer);
                        nk_buffer_init_fixed(&ebuf, index_data, max_index_buffer);
                        nk_convert(&m_nk_context, &m_nk_commands, &vbuf, &ebuf, &config);
                    }
                );
            }
        );

        gfx::Renderer_Base& renderer = m_rendering_module->renderer_base();

        // Draw each draw command
        nk_size offset = 0;
        const nk_draw_command* cmd = nullptr;
        nk_draw_foreach(cmd, &m_nk_context, &m_nk_commands)
        {
            if (!cmd->elem_count)
                continue;

            renderer.bind_texture(cmd->texture.id, 0);

            const v2i scissor_pos {
                cmd->clip_rect.x * framebuffer_scale.x,
                (f32(window_size.y) - (cmd->clip_rect.y + cmd->clip_rect.h)) * framebuffer_scale.y };
            const v2i scissor_size {
                cmd->clip_rect.w * framebuffer_scale.x,
                cmd->clip_rect.h * framebuffer_scale.y };
            renderer.set_scissor(scissor_pos, scissor_size);

            renderer.draw_elements(*m_nuklear_vertex_array, cmd->elem_count, gfx::AttributeType::U16, offset);

            offset += cmd->elem_count * sizeof(nk_draw_index);
        }
    }
}