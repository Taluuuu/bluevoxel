#include "renderer_opengl.h"

#include "core/log.h"
#include "core/game_info.h"
#include "rendering/pipeline.h"
#include "windowing/window.h"
#include "pipeline_opengl.h"
#include "buffer_opengl.h"
#include "vertex_array_opengl.h"
#include "texture_opengl.h"

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
        m_bound_pipeline = std::dynamic_pointer_cast<Pipeline_OpenGL>(pipeline);
        if (m_bound_pipeline)
            glUseProgram(m_bound_pipeline->handle());
    }

    std::shared_ptr<IBuffer> Renderer_OpenGL::create_buffer()
    {
        return std::make_shared<Buffer_OpenGL>();
    }

    std::shared_ptr<IVertexArray> Renderer_OpenGL::create_vertex_array()
    {
        return std::make_shared<VertexArray_OpenGL>();
    }

    std::shared_ptr<ITexture> Renderer_OpenGL::create_texture(const std::string& path)
    {
        return Texture_OpenGL::create(path);
    }

    void Renderer_OpenGL::draw(const IVertexArray& vertex_array, i32 count)
    {
        if (!m_bound_pipeline)
            return;

        auto vertex_array_gl = dynamic_cast<const VertexArray_OpenGL*>(&vertex_array);
        assert(vertex_array_gl);

        vertex_array_gl->bind();
        glDrawArrays(GL_TRIANGLES, 0, count);
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
        glClearColor(0.0f, 0.1f, 0.2f, 1.0f);
        glEnable(GL_DEBUG_OUTPUT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

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
}