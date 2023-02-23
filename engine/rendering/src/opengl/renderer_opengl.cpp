#include "renderer_opengl.h"

#include "core/log.h"
#include "rendering/pipeline.h"
#include "windowing/window.h"

#include <glad/gl.h>
#include <glfw/glfw3.h>

namespace engine
{
    PipelineCreateData Renderer_OpenGL::create_pipeline()
    {
        return PipelineCreateData(*this);
    }

    std::shared_ptr<IPipeline> Renderer_OpenGL::compile_pipeline(const PipelineCreateData& create_data)
    {
        return nullptr;
    }

    void Renderer_OpenGL::bind_pipeline(const std::shared_ptr<IPipeline>& pipeline)
    {

    }

    bool Renderer_OpenGL::init(const GameInfo& game_info, IWindow& window)
    {
        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0)
        {
            log::error("Failed to initialize OpenGL context.");
            return false;
        }

        log::info("Loaded OpenGL {}.{}.", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

        glViewport(0, 0, window.framebuffer_size().x, window.framebuffer_size().y);
        glClearColor(0.0f, 0.1f, 0.2f, 1.0f);

        return true;
    }

    void Renderer_OpenGL::draw_frame()
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}