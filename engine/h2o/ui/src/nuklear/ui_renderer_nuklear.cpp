#include "ui_renderer_nuklear.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "nuklear_utils.h"
#include "rendering/buffer.h"
#include "rendering/pipeline.h"
#include "rendering/renderer_base.h"
#include "rendering/rendering_module.h"
#include "rendering/texture.h"
#include "windowing/windowing_module.h"

#define NK_IMPLEMENTATION
#include "nuklear_headers.h"

namespace h2o
{
    bool UIRenderer_Nuklear::init(Engine& engine)
    {
        m_input_module     = &engine.get_module_checked<InputModule>();
        m_rendering_module = &engine.get_module_checked<RenderingModule>();
        m_windowing_module = &engine.get_module_checked<WindowingModule>();

        const auto& rendering_module = engine.get_module_checked<RenderingModule>();
        gfx::Renderer_Base& renderer = rendering_module.renderer_base();

        nk_init_default(&m_nk_ctx, nullptr);
        nk_buffer_init_default(&m_nk_commands);

        // Setup Nuklear rendering pipeline
        m_nuklear_pipeline = renderer
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex, "engine/shaders/nuklear_ui.vert")
            .add_shader(gfx::ShaderStage::Fragment, "engine/shaders/nuklear_ui.frag")
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
        m_nuklear_index_buffer->update_data(nullptr, max_index_buffer, gfx::BufferUsage::DynamicDraw);
        m_nuklear_vertex_buffer = renderer.create_buffer_ptr();
        m_nuklear_vertex_buffer->update_data(nullptr, max_vertex_buffer, gfx::BufferUsage::DynamicDraw);

        m_nuklear_vertex_array->attach_index_buffer(m_nuklear_index_buffer);
        m_nuklear_vertex_array->attach_vertex_buffer(m_nuklear_vertex_buffer, 0, 0, sizeof(NkVertex));
        m_nuklear_vertex_array->setup_attribute_float(m_attrib_pos, 0, gfx::AttributeType::F32, false, 2, offsetof(NkVertex, position));
        m_nuklear_vertex_array->setup_attribute_float(m_attrib_uv,  0, gfx::AttributeType::F32, false, 2, offsetof(NkVertex, uv));
        m_nuklear_vertex_array->setup_attribute_float(m_attrib_col, 0, gfx::AttributeType::U8, true, 4, offsetof(NkVertex, col));

        nk_font_atlas_init_default(&m_nk_atlas);
        nk_font_atlas_begin(&m_nk_atlas);

        // Bake default font
        v2i image_size{};
        const void* image = nk_font_atlas_bake(&m_nk_atlas, &image_size.x, &image_size.y, NK_FONT_ATLAS_RGBA32);
        m_font_texture = renderer.create_texture_ptr();
        m_font_texture->update_data({ image_size, 4 }, image);

        nk_font_atlas_end(&m_nk_atlas, nk_handle_id(m_font_texture->id()), &m_nk_texture_null);
        nk_style_set_font(&m_nk_ctx, &m_nk_atlas.default_font->handle);

        m_windowing_module->window().char_event().add_listener(m_char_event_handle,
            [this](const CharEvent& char_event)
            {
                m_pressed_unicode_chars.push_back(char_event.unicode_char);
            }
        );

        return true;
    }

    void UIRenderer_Nuklear::cleanup()
    {

    }

    void UIRenderer_Nuklear::frame_start()
    {
        nk_input_begin(&m_nk_ctx);

        if (!m_input_module->is_mouse_captured())
        {
            nk_input_key(&m_nk_ctx, NK_KEY_DEL, m_input_module->key_state(Key::Delete).held);
            nk_input_key(&m_nk_ctx, NK_KEY_ENTER, m_input_module->key_state(Key::Enter).held);
            nk_input_key(&m_nk_ctx, NK_KEY_TAB, m_input_module->key_state(Key::Tab).held);
            nk_input_key(&m_nk_ctx, NK_KEY_BACKSPACE, m_input_module->key_state(Key::Backspace).held);
            nk_input_key(&m_nk_ctx, NK_KEY_UP, m_input_module->key_state(Key::Up).held);
            nk_input_key(&m_nk_ctx, NK_KEY_DOWN, m_input_module->key_state(Key::Down).held);
            nk_input_key(&m_nk_ctx, NK_KEY_LEFT, m_input_module->key_state(Key::Left).held);
            nk_input_key(&m_nk_ctx, NK_KEY_RIGHT, m_input_module->key_state(Key::Right).held);
            nk_input_key(&m_nk_ctx, NK_KEY_TEXT_START, m_input_module->key_state(Key::Home).held);
            nk_input_key(&m_nk_ctx, NK_KEY_TEXT_END, m_input_module->key_state(Key::End).held);
            nk_input_key(&m_nk_ctx, NK_KEY_SCROLL_START, m_input_module->key_state(Key::Home).held);
            nk_input_key(&m_nk_ctx, NK_KEY_SCROLL_END, m_input_module->key_state(Key::End).held);
            nk_input_key(&m_nk_ctx, NK_KEY_SCROLL_DOWN, m_input_module->key_state(Key::PageDown).held);
            nk_input_key(&m_nk_ctx, NK_KEY_SCROLL_UP, m_input_module->key_state(Key::PageUp).held);
            nk_input_key(&m_nk_ctx, NK_KEY_SHIFT,
                m_input_module->key_state(Key::LeftShift).held || m_input_module->key_state(Key::RightShift).held);

            for (u32 unicode_char: m_pressed_unicode_chars)
                nk_input_unicode(&m_nk_ctx, unicode_char);
            m_pressed_unicode_chars.clear();

            // Mouse input
            const v2i mouse_position = m_input_module->mouse_position();
            nk_input_motion(&m_nk_ctx, mouse_position.x, mouse_position.y);
            nk_input_button(&m_nk_ctx, NK_BUTTON_LEFT,
                mouse_position.x, mouse_position.y,
                m_input_module->mouse_button_state(MouseButton::Left).held);
        }

        nk_input_end(&m_nk_ctx);
    }

    void UIRenderer_Nuklear::frame_end()
    {
        const IWindow& window = m_windowing_module->window();

        const v2i window_size = window.window_size();
        const v2i framebuffer_size = window.framebuffer_size();
        const v2 framebuffer_scale = v2(framebuffer_size) / v2(window_size);

        const m4 ortho {
            2.0f / f32(window_size.x),  0.0f,                       0.0f,  0.0f,
            0.0f,                      -2.0f / f32(window_size.y),  0.0f,  0.0f,
            0.0f,                       0.0f,                      -1.0f,  0.0f,
            -1.0f,                      1.0f,                       0.0f,  1.0f
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
                .global_alpha = 1.0f,
                .line_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON,
                .shape_AA = nk_anti_aliasing::NK_ANTI_ALIASING_ON,
                .circle_segment_count = 3,
                .arc_segment_count = 3,
                .curve_segment_count = 3,
                .tex_null = m_nk_texture_null,
                .vertex_layout = vertex_layout,
                .vertex_size = sizeof(NkVertex),
                .vertex_alignment = NK_ALIGNOF(NkVertex),
            };

        m_nuklear_vertex_buffer->map_write_only(
            [&](void* vertex_data)
            {
                m_nuklear_index_buffer->map_write_only(
                    [&](void* index_data)
                    {
                        nk_buffer_init_fixed(&vbuf, vertex_data, max_vertex_buffer);
                        nk_buffer_init_fixed(&ebuf, index_data, max_index_buffer);
                        nk_convert(&m_nk_ctx, &m_nk_commands, &vbuf, &ebuf, &config);
                    }
                );
            }
        );

        gfx::Renderer_Base& renderer = m_rendering_module->renderer_base();
        renderer.bind_pipeline(m_nuklear_pipeline);

        // Draw each draw command
        nk_size offset = 0;
        const nk_draw_command* cmd;
        nk_draw_foreach(cmd, &m_nk_ctx, &m_nk_commands)
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

        m_is_mouse_over_ui = nk_item_is_any_active(&m_nk_ctx);

        nk_clear(&m_nk_ctx);
        nk_buffer_clear(&m_nk_commands);
    }

    bool UIRenderer_Nuklear::is_mouse_over_ui() const
    {
        return m_is_mouse_over_ui;
    }

    bool UIRenderer_Nuklear::is_mouse_over_next_widget() const
    {
        return nk_widget_is_hovered(const_cast<nk_context*>(&m_nk_ctx));
    }

    void UIRenderer_Nuklear::row(f32 height, i32 num_columns)
    {
        nk_layout_row_dynamic(&m_nk_ctx, height, num_columns);
    }

    void UIRenderer_Nuklear::label(const std::string& label)
    {
        nk_label(&m_nk_ctx, label.c_str(), NK_TEXT_LEFT);
    }

    bool UIRenderer_Nuklear::button(const std::string& title)
    {
        return nk_button_label(&m_nk_ctx, title.c_str());
    }

    bool UIRenderer_Nuklear::input_text(const std::string& label, std::string& text)
    {
        // Super scuffed
        static constexpr size_t max_len = 100;
        i32 text_len = i32(text.size());
        assert(text_len + 1 <= max_len);

        char temp[max_len];
        strcpy_s(temp, text_len + 1, text.c_str());

        if (nk_edit_string(
            &m_nk_ctx, NK_EDIT_FIELD,
            temp, &text_len, max_len,
            nk_filter_ascii) != NK_EDIT_ACTIVE)
        {
            return false;
        }

        text.clear();
        text.append(temp, text_len);

        return true;
    }

    bool UIRenderer_Nuklear::input_int(const std::string& label, i32& num)
    {
        const i32 prev_val = num;
        nk_property_int(&m_nk_ctx, label.c_str(), 0, &num, 9999999, 1, 0.0f);
        return num != prev_val;
    }

    bool UIRenderer_Nuklear::input_uint(const std::string& label, u32& num)
    {
        i32 current_val = num;
        nk_property_int(&m_nk_ctx, label.c_str(), 0, &current_val, 9999999, 1, 0.0f);

        if (current_val != num)
        {
            num = current_val;
            return true;
        }

        return false;
    }

    bool UIRenderer_Nuklear::combobox(const std::vector<const char*>& options, u32& selected_index)
    {
        i32 current_index_value = static_cast<i32>(selected_index);
        nk_combobox(&m_nk_ctx,
            const_cast<const char**>(options.data()), static_cast<i32>(options.size()),
            &current_index_value, 20, { 250.0f, 250.0f });

        if (selected_index != current_index_value)
        {
            selected_index = current_index_value;
            return true;
        }

        return false;
    }

    void UIRenderer_Nuklear::group(const std::string& title, const std::function<void()>& group_contents)
    {
        if (nk_group_begin(&m_nk_ctx, title.c_str(), 0))
        {
            group_contents();
            nk_group_end(&m_nk_ctx);
        }
    }

    void UIRenderer_Nuklear::tree_push(const std::string& title, const std::function<void()>& tree_contents)
    {
        if (nk_tree_push(&m_nk_ctx, NK_TREE_TAB, title.c_str(), NK_MAXIMIZED))
        {
            tree_contents();

            nk_tree_pop(&m_nk_ctx);
        }
    }

    bool UIRenderer_Nuklear::window_begin(
        const std::string& title,
        const ui::Rect& rect)
    {
        return nk_begin(
            &m_nk_ctx, title.c_str(), ui::to_nk_rect(rect),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
            NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE);
    }

    void UIRenderer_Nuklear::window_end()
    {
        nk_end(&m_nk_ctx);
    }
}