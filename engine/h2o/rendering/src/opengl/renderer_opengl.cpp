#include "renderer_opengl.h"

#include "core/log.h"
#include "core/engine.h"
#include "core/game_info.h"
#include "rendering/pipeline.h"
#include "windowing/window.h"
#include "pipeline_opengl.h"
#include "buffer_opengl.h"
#include "rendering/mesh.h"
#include "texture_opengl.h"
#include "texture_array_opengl.h"
#include "vertex_array_opengl.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#if H2O_USE_GLFW
#   include <backends/imgui_impl_glfw.h>
#else
#   error "Only GLFW backend is supported for ImGUI at the moment."
#endif

namespace h2o::gfx
{
    PipelineCreateData Renderer_OpenGL::create_pipeline()
    {
        return PipelineCreateData(*this);
    }

    std::shared_ptr<IPipeline> Renderer_OpenGL::compile_pipeline(const PipelineCreateData& create_data)
    {
        return Pipeline_OpenGL::create(create_data);
    }

    void Renderer_OpenGL::bind_pipeline(const std::shared_ptr<IPipeline>& pipeline)
    {
        // TODO: Shader pipelines should use the resource manager
        m_bound_pipeline = std::dynamic_pointer_cast<Pipeline_OpenGL>(pipeline);
        if (m_bound_pipeline)
            glUseProgram(m_bound_pipeline->handle());
    }

    // TODO: This implementation and ones like it could be moved to Renderer_Base
    Buffer Renderer_OpenGL::create_buffer()
    {
        return Buffer(*this);
    }

    std::shared_ptr<Buffer> Renderer_OpenGL::create_buffer_ptr()
    {
        return std::make_shared<Buffer>(*this);
    }

    std::shared_ptr<IBuffer> Renderer_OpenGL::create_buffer_OLD()
    {
        return std::make_shared<Buffer_OpenGL>();
    }

    VertexArray Renderer_OpenGL::create_vertex_array()
    {
        return VertexArray(*this);
    }

    std::shared_ptr<IVertexArray> Renderer_OpenGL::create_vertex_array_OLD()
    {
        return std::make_shared<VertexArray_OpenGL>();
    }

    std::shared_ptr<ITexture> Renderer_OpenGL::fetch_or_load_texture(const std::string& path)
    {
        return g_engine->resource_mgr().fetch<Texture_OpenGL>(path);
    }

    std::shared_ptr<ITextureArray> Renderer_OpenGL::create_texture_array(size_t array_size)
    {
        return std::make_shared<TextureArray_OpenGL>(array_size);
    }

    void Renderer_OpenGL::draw(const IVertexArray& vertex_array, i32 count)
    {
        if (!m_bound_pipeline || count == 0)
            return;

        auto vertex_array_gl = dynamic_cast<const VertexArray_OpenGL*>(&vertex_array);
        assert(vertex_array_gl);

        vertex_array_gl->bind();
        glDrawArrays(GL_TRIANGLES, 0, count);
    }

    void Renderer_OpenGL::draw(const VertexArray& vertex_array, u32 vertex_count)
    {
        if (!m_bound_pipeline || vertex_count == 0)
            return;

        glBindVertexArray(vertex_array.id());
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(vertex_count));
    }

    void Renderer_OpenGL::draw(const Mesh& mesh)
    {
        if (!m_bound_pipeline)
            return;

        auto vertex_array_gl = dynamic_cast<const VertexArray_OpenGL*>(mesh.vertex_array().get());
        assert(vertex_array_gl);

        vertex_array_gl->bind();
        glDrawElements(GL_TRIANGLES, mesh.vertex_count(), GL_UNSIGNED_INT, nullptr);
    }

    bool Renderer_OpenGL::init(IWindow& window, const GameInfo& game_info)
    {
        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0)
        {
            log::error("Failed to initialize OpenGL context.");
            return false;
        }

        log::info("Loaded OpenGL {}.{}.", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        glViewport(0, 0, window.window_size().x, window.window_size().y);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        // glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        window.resize_event().add_listener(m_window_resize_event_handle,
            [](const WindowResizeEvent& event)
            {
                glViewport(0, 0, static_cast<i32>(event.new_size.x), static_cast<i32>(event.new_size.y));
            });

        // Init ImGui
        ImGui::CreateContext();

#if H2O_USE_GLFW
        // TODO: Move this to windowing module ??
        if (!ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window.handle()), true))
        {
            log::error("Failed to initialize ImGui GLFW.");
            return false;
        }
#endif

#if H2O_USE_OPENGL
        if (!ImGui_ImplOpenGL3_Init())
        {
            log::error("Failed to initialize ImGui OpenGL3.");
            return false;
        }
#endif

        return true;
    }

    void Renderer_OpenGL::start_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if H2O_USE_OPENGL
        ImGui_ImplOpenGL3_NewFrame();
#endif

#if H2O_USE_GLFW
        ImGui_ImplGlfw_NewFrame();
#endif

        ImGui::NewFrame();
    }

    void Renderer_OpenGL::end_frame()
    {
        ImGui::Render();

#if H2O_USE_OPENGL
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
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

    void Renderer_OpenGL::update_buffer_data(Buffer& buffer, const void* data, size_t size)
    {
        assert(buffer.is_valid());
        glNamedBufferData(buffer.id(), GLsizeiptr(size), data, GL_STATIC_DRAW);
    }
}