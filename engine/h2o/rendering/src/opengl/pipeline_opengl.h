#pragma once

#include "headers_opengl.h"
#include "rendering/pipeline.h"

namespace h2o::gfx
{
    class Pipeline_OpenGL : public IPipeline
    {
    public:

        static std::shared_ptr<Pipeline_OpenGL> create(const PipelineCreateData& create_data);
        Pipeline_OpenGL(const Pipeline_OpenGL&) = delete;
        Pipeline_OpenGL(Pipeline_OpenGL&& other) noexcept;
        ~Pipeline_OpenGL() override;

    public:

        // IPipeline interface
        void set_uniform_mat4(i32 location, const m4& value) const override;
        void set_uniform_vec3(i32 location, const v3& value) const override;
        void set_uniform_vec4(i32 location, const v4& value) const override;
        void set_uniform_ivec3(i32 location, const v3i& value) const override;
        void set_uniform_float(i32 location, f32 value) const override;
        void set_uniform_int(i32 location, i32 value) const override;
        i32 get_uniform_location(const std::string& name) const override;
        i32 get_attribute_location(const std::string& name) const override;
        const PipelineConfig& pipeline_config() const override;

        [[nodiscard]] GLuint handle() const { return m_program; }

    private:

        Pipeline_OpenGL(GLuint program, const PipelineConfig& pipeline_config);

    private:

        PipelineConfig m_pipeline_config{};

        GLuint m_program = 0;

    };
}