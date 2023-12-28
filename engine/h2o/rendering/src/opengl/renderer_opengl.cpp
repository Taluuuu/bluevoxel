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
        case AttributeType::U16: return GL_UNSIGNED_SHORT;
        case AttributeType::U32: return GL_UNSIGNED_INT;
        case AttributeType::F32: return GL_FLOAT;
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

    void Renderer_OpenGL::draw_arrays(const VertexArray& vertex_array, u32 vertex_count)
    {
        if (!m_bound_pipeline || vertex_count == 0)
            return;

        glBindVertexArray(vertex_array.id());
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertex_count));
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

        return true;
    }

    void Renderer_OpenGL::start_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer_OpenGL::end_frame()
    {

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

    void Renderer_OpenGL::attach_vertex_buffer(VertexArray& vertex_array, const Buffer& buffer, u32 binding_index, i64 offset, i32 stride)
    {
        assert(vertex_array.is_valid());
        assert(buffer.is_valid());
        glVertexArrayVertexBuffer(vertex_array.id(), binding_index, buffer.id(), offset, stride);
    }

    void Renderer_OpenGL::update_index_buffer(VertexArray& vertex_array, const Buffer& buffer)
    {
        assert(vertex_array.is_valid());
        assert(buffer.is_valid());
        glVertexArrayElementBuffer(vertex_array.id(), buffer.id());
    }

    void Renderer_OpenGL::setup_attribute(VertexArray& vertex_array, u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset)
    {
        assert(vertex_array.is_valid());

        glEnableVertexArrayAttrib(vertex_array.id(), attribute_index);

        switch (type)
        {
        case AttributeType::F32:
            glVertexArrayAttribFormat(vertex_array.id(), attribute_index, size, GL_FLOAT, GL_FALSE, relative_offset);
            break;
        case AttributeType::U32:
            glVertexArrayAttribIFormat(vertex_array.id(), attribute_index, size, GL_UNSIGNED_INT, relative_offset);
            break;
        default:
            assert(false);
        }

        glVertexArrayAttribBinding(vertex_array.id(), attribute_index, binding_index);
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

    void Renderer_OpenGL::update_buffer_data(Buffer& buffer, const void* data, size_t size, BufferUsage buffer_usage)
    {
        assert(buffer.is_valid());

        GLenum usage = GL_STATIC_DRAW;
        switch (buffer_usage)
        {
        case BufferUsage::StreamDraw: usage = GL_STREAM_DRAW; break;
        case BufferUsage::StaticDraw: usage = GL_STATIC_DRAW; break;
        }

        glNamedBufferData(buffer.id(), GLsizeiptr(size), data, usage);
    }

    void Renderer_OpenGL::destroy_texture(u32 texture_id)
    {
        glDeleteTextures(1, &texture_id);
    }

    void Renderer_OpenGL::bind_texture(Texture& texture, u32 texture_slot)
    {
        glBindTextureUnit(texture_slot, texture.id());
    }

    u32 Renderer_OpenGL::allocate_texture()
    {
        u32 handle;
        glCreateTextures(GL_TEXTURE_2D, 1, &handle);
        return handle;
    }

    void Renderer_OpenGL::update_texture_data(Texture& texture, const TextureFormat& format, const void* data)
    {
        glTextureParameteri(texture.id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(texture.id(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture.id(), GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture.id(), GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTextureStorage2D(texture.id(), 1, GL_RGBA8, i32(format.size.x), i32(format.size.y));
        glTextureSubImage2D(texture.id(), 0, 0, 0, i32(format.size.x), i32(format.size.y), GL_RGBA, GL_UNSIGNED_BYTE, data);
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

    void Renderer_OpenGL::init_texture_array(TextureArray& texture_array, const TextureFormat& texture_format)
    {
        const u32 handle = texture_array.id();
        assert(texture_format.nb_channels == 4); // TODO: Support more formats later

        glTextureStorage3D(handle, 1, GL_RGBA8,
            GLsizei(texture_format.size.x),
            GLsizei(texture_format.size.y),
            GLsizei(texture_array.size()));

        // Set texture parameters
        glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    void Renderer_OpenGL::bind_texture_array(TextureArray& texture_array, u32 texture_slot)
    {
        glBindTextureUnit(texture_slot, texture_array.id());
    }

    void Renderer_OpenGL::attach_texture_to_texture_array(TextureArray& texture_array, const Texture& texture, u32 index)
    {
        glCopyImageSubData(
            texture.id(),       GL_TEXTURE_2D,       0, 0, 0, 0,
            texture_array.id(), GL_TEXTURE_2D_ARRAY, 0, 0, 0, GLint(index),
            GLsizei(texture.format().size.x), GLsizei(texture.format().size.y), 1);
    }
}