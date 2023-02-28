#pragma once

#include "headers_opengl.h"
#include "rendering/pipeline.h"

namespace engine
{
    class Pipeline_OpenGL : public IPipeline
    {
    public:

        static std::shared_ptr<Pipeline_OpenGL> create(const PipelineCreateData& create_data);
        Pipeline_OpenGL(const Pipeline_OpenGL&) = delete;
        Pipeline_OpenGL(Pipeline_OpenGL&& other) noexcept;
        ~Pipeline_OpenGL() override;
    private:
        explicit Pipeline_OpenGL(GLuint program);

    public:

        // IPipeline interface
        void set_uniform_mat4(i32 location, const m4& value) override;

        [[nodiscard]] GLuint handle() const { return m_program; }

    private:

        GLuint m_program = 0;

    };
}