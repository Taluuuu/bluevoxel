#include "renderer_opengl.h"

#include "core/log.h"
#include "core/game_info.h"
#include "rendering/pipeline.h"
#include "windowing/window.h"
#include "pipeline_opengl.h"
#include "buffer_opengl.h"
#include "vertex_array_opengl.h"

namespace engine
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

    void Renderer_OpenGL::draw(const IVertexArray& vertex_array)
    {
        if (!m_bound_pipeline)
            return;

        auto vertex_array_gl = dynamic_cast<const VertexArray_OpenGL*>(&vertex_array);
        assert(vertex_array_gl);

        vertex_array_gl->bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        return true;
    }

    void Renderer_OpenGL::clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}