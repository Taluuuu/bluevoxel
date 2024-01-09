#include "renderer_opengl.h"

#include "core/engine.h"
#include "core/game_info.h"
#include "core/log.h"
#include "pipeline_opengl.h"
#include "rendering/buffer.h"
#include "rendering/mesh.h"
#include "rendering/pipeline.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "rendering/vertex_array.h"
#include "windowing/window.h"

namespace h2o::gfx
{
    static constexpr GLenum to_gl_blend_equation(BlendEquation blend_equation)
    {
        switch (blend_equation)
        {
        case BlendEquation::Add: return GL_FUNC_ADD;
        }

        assert(false);
        return {};
    }

    static constexpr GLenum to_gl_blend_factor(BlendFactor blend_factor)
    {
        switch (blend_factor)
        {
        case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        }

        assert(false);
        return {};
    }

    static constexpr GLenum to_gl_attribute_type(AttributeType attribute_type)
    {
        switch (attribute_type)
        {
        case AttributeType::U8:  return GL_UNSIGNED_BYTE;
        case AttributeType::U16: return GL_UNSIGNED_SHORT;
        case AttributeType::U32: return GL_UNSIGNED_INT;
        case AttributeType::F32: return GL_FLOAT;
        }

        assert(false);
        return {};
    }

    static constexpr GLenum to_gl_draw_mode(DrawMode draw_mode)
    {
        switch (draw_mode)
        {
        case DrawMode::Triangles: return GL_TRIANGLES;
        case DrawMode::Lines:     return GL_LINES;
        }

        assert(false);
        return {};
    }

    std::shared_ptr<IPipeline> Renderer_OpenGL::compile_pipeline(const PipelineCreateData& create_data)
    {
        return Pipeline_OpenGL::create(create_data);
    }

    void Renderer_OpenGL::bind_pipeline(const std::shared_ptr<IPipeline>& pipeline)
    {
        m_bound_pipeline = std::dynamic_pointer_cast<Pipeline_OpenGL>(pipeline);
        if (!m_bound_pipeline)
            return;

        glUseProgram(m_bound_pipeline->handle());

        const auto& pipeline_cfg = m_bound_pipeline->pipeline_config();

        // Enable/disable GL features
        if (pipeline_cfg.pipeline_features & PipelineFeature::Blend) glEnable(GL_BLEND);
        else glDisable(GL_BLEND);
        if (pipeline_cfg.pipeline_features & PipelineFeature::CullFace) glEnable(GL_CULL_FACE);
        else glDisable(GL_CULL_FACE);
        if (pipeline_cfg.pipeline_features & PipelineFeature::DepthTest) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        if (pipeline_cfg.pipeline_features & PipelineFeature::ScissorTest) glEnable(GL_SCISSOR_TEST);
        else glDisable(GL_SCISSOR_TEST);

        // Blending
        glBlendEquation(to_gl_blend_equation(pipeline_cfg.blend_equation));
        glBlendFunc(
            to_gl_blend_factor(pipeline_cfg.blend_source_factor),
            to_gl_blend_factor(pipeline_cfg.blend_dest_factor));
    }

    void Renderer_OpenGL::draw_arrays(const VertexArray& vertex_array, u32 vertex_count, DrawMode draw_mode)
    {
        if (!m_bound_pipeline || vertex_count == 0)
            return;

        glBindVertexArray(vertex_array.id());
        glDrawArrays(to_gl_draw_mode(draw_mode), 0, GLsizei(vertex_count));
    }

    void Renderer_OpenGL::draw_elements(const VertexArray& vertex_array, u32 vertex_count, AttributeType indices_type, u64 byte_offset)
    {
        if (!m_bound_pipeline || vertex_count == 0)
            return;

        glBindVertexArray(vertex_array.id());
        glDrawElements(GL_TRIANGLES, GLsizei(vertex_count), to_gl_attribute_type(indices_type), (void*)byte_offset);
    }

    void Renderer_OpenGL::draw(const Mesh& mesh)
    {
        assert(mesh.vertex_array());
        draw_elements(*mesh.vertex_array(), mesh.vertex_count(), AttributeType::U32, 0);
    }

    bool Renderer_OpenGL::init(IWindow& window, const GameInfo& game_info)
    {
        const int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0)
        {
            log::error("Failed to initialize OpenGL context.");
            return false;
        }

        log::info("Loaded OpenGL {}.{}.", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        glViewport(0, 0, window.window_size().x, window.window_size().y);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

        window.resize_event().add_listener(m_window_resize_event_handle,
            [](const WindowResizeEvent& event)
            {
                glViewport(0, 0, static_cast<i32>(event.new_size.x), static_cast<i32>(event.new_size.y));
            });

        return Renderer_Base::init(window, game_info);
    }

    void Renderer_OpenGL::cleanup()
    {
        Renderer_Base::cleanup();
    }

    void Renderer_OpenGL::start_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Renderer_Base::start_frame();
    }

    void Renderer_OpenGL::end_frame()
    {
        Renderer_Base::end_frame();
    }

    void Renderer_OpenGL::set_clear_color(const v4& color)
    {
        glClearColor(color.x, color.y, color.z, color.w);
    }

    void Renderer_OpenGL::set_scissor(v2i scissor_pos, v2i scissor_size)
    {
        glScissor(scissor_pos.x, scissor_pos.y, scissor_size.x, scissor_size.y);
    }

    u32 Renderer_OpenGL::allocate_vertex_array()
    {
        GLuint vao;
        glCreateVertexArrays(1, &vao);
        return vao;
    }

    void Renderer_OpenGL::destroy_vertex_array(u32 vertex_array_id)
    {
        glDeleteVertexArrays(1, &vertex_array_id);
    }

    void Renderer_OpenGL::attach_vertex_buffer(u32 vertex_array_id, const Buffer& buffer, u32 binding_index, i64 offset, i32 stride)
    {
        assert(buffer.is_valid());
        glVertexArrayVertexBuffer(vertex_array_id, binding_index, buffer.id(), offset, stride);
    }

    void Renderer_OpenGL::update_index_buffer(u32 vertex_array_id, const Buffer& buffer)
    {
        assert(buffer.is_valid());
        glVertexArrayElementBuffer(vertex_array_id, buffer.id());
    }

    void Renderer_OpenGL::setup_attribute_float(u32 vertex_array_id, u32 attribute_index, u32 binding_index, AttributeType type, bool normalize, i32 size, u32 relative_offset)
    {
        glEnableVertexArrayAttrib(vertex_array_id, attribute_index);
        glVertexArrayAttribFormat(vertex_array_id, attribute_index, size, to_gl_attribute_type(type), normalize, relative_offset);
        glVertexArrayAttribBinding(vertex_array_id, attribute_index, binding_index);
    }

    void Renderer_OpenGL::setup_attribute_int(u32 vertex_array_id, u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset)
    {
        glEnableVertexArrayAttrib(vertex_array_id, attribute_index);
        glVertexArrayAttribIFormat(vertex_array_id, attribute_index, size, to_gl_attribute_type(type), relative_offset);
        glVertexArrayAttribBinding(vertex_array_id, attribute_index, binding_index);
    }

    u32 Renderer_OpenGL::allocate_buffer()
    {
        GLuint vbo;
        glCreateBuffers(1, &vbo);
        return vbo;
    }

    void Renderer_OpenGL::destroy_buffer(u32 buffer_id)
    {
        glDeleteBuffers(1, &buffer_id);
    }

    void Renderer_OpenGL::update_buffer_data(u32 buffer_id, const void* data, size_t size, BufferUsage buffer_usage)
    {
        GLenum usage = GL_STATIC_DRAW;
        switch (buffer_usage)
        {
        case BufferUsage::StreamDraw:  usage = GL_STREAM_DRAW;  break;
        case BufferUsage::StaticDraw:  usage = GL_STATIC_DRAW;  break;
        case BufferUsage::DynamicDraw: usage = GL_DYNAMIC_DRAW; break;
        }

        glNamedBufferData(buffer_id, GLsizeiptr(size), data, usage);
    }

    const void* Renderer_OpenGL::map_buffer_read_only(u32 buffer_id)
    {
        return glMapNamedBuffer(buffer_id, GL_READ_ONLY);
    }

    void* Renderer_OpenGL::map_buffer_write_only(u32 buffer_id)
    {
        return glMapNamedBuffer(buffer_id, GL_WRITE_ONLY);
    }

    void* Renderer_OpenGL::map_buffer_read_write(u32 buffer_id)
    {
        return glMapNamedBuffer(buffer_id, GL_READ_WRITE);
    }

    void Renderer_OpenGL::unmap_buffer(u32 buffer_id)
    {
        glUnmapNamedBuffer(buffer_id);
    }

    void Renderer_OpenGL::destroy_texture(u32 texture_id)
    {
        glDeleteTextures(1, &texture_id);
    }

    void Renderer_OpenGL::bind_texture(u32 texture_id, u32 texture_slot)
    {
        glBindTextureUnit(texture_slot, texture_id);
    }

    u32 Renderer_OpenGL::allocate_texture()
    {
        u32 handle;
        glCreateTextures(GL_TEXTURE_2D, 1, &handle);
        return handle;
    }

    void Renderer_OpenGL::update_texture_data(u32 texture_id, const TextureFormat& format, const void* data)
    {
        glTextureParameteri(texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureStorage2D(texture_id, 1, GL_RGBA8, i32(format.size.x), i32(format.size.y));
        glTextureSubImage2D(texture_id, 0, 0, 0, i32(format.size.x), i32(format.size.y), GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    u32 Renderer_OpenGL::allocate_texture_array()
    {
        u32 handle;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &handle);
        return handle;
    }

    void Renderer_OpenGL::destroy_texture_array(u32 texture_array_id)
    {
        glDeleteTextures(1, &texture_array_id);
    }

    void Renderer_OpenGL::init_texture_array(u32 texture_array_id, u32 array_size, const TextureFormat& texture_format)
    {
        assert(texture_format.nb_channels == 4); // TODO: Support more formats later

        glTextureStorage3D(texture_array_id, 1, GL_RGBA8,
            GLsizei(texture_format.size.x),
            GLsizei(texture_format.size.y),
            GLsizei(array_size));

        // Set texture parameters
        glTextureParameteri(texture_array_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture_array_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture_array_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture_array_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void Renderer_OpenGL::bind_texture_array(u32 texture_array_id, u32 texture_slot)
    {
        glBindTextureUnit(texture_slot, texture_array_id);
    }

    void Renderer_OpenGL::attach_texture_to_texture_array(u32 texture_array_id, const Texture& texture, u32 index)
    {
        glCopyImageSubData(
            texture.id(),     GL_TEXTURE_2D,       0, 0, 0, 0,
            texture_array_id, GL_TEXTURE_2D_ARRAY, 0, 0, 0, GLint(index),
            GLsizei(texture.format().size.x), GLsizei(texture.format().size.y), 1);
    }
}