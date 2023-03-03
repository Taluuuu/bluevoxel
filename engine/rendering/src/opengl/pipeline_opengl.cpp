#include "pipeline_opengl.h"

#include "core/log.h"
#include "core/utils.h"

#include <cassert>
#include <glm/gtc/type_ptr.hpp>

namespace h2o::gfx
{
    static GLuint compile_shader(
        const std::string& path, GLuint type)
    {
        // Load file
        auto code = utils::read_file(path);
        if (!code.has_value())
        {
            log::error("Failed to load shader source file: {}", path);
            return 0;
        }

        // Create and compile shader
        GLuint shader = glCreateShader(type);
        const GLchar* code_c_str = code->data();
        glShaderSource(shader, 1, &code_c_str, nullptr);
        glCompileShader(shader);

        // Check shader compilation
        GLint success;
        GLchar info_log[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 512, nullptr, info_log);
            log::error("Failed to compile shader at path: '{}'. Info log: \n{}", path, info_log);
            glDeleteShader(shader); // Delete shader that failed to compile
            return 0;
        }

        return shader;
    }

    std::shared_ptr<Pipeline_OpenGL> Pipeline_OpenGL::create(const PipelineCreateData& create_data)
    {
        const auto& vert_shader_create_data = create_data.vertex_shader();
        const auto& frag_shader_create_data = create_data.fragment_shader();
        const auto& geom_shader_create_data = create_data.geometry_shader();
        bool use_geometry_shader = geom_shader_create_data.has_value();
        // This should call a different function if there is no geometry shader to avoid all the if (use_geometry_shader)s

        if (!vert_shader_create_data.has_value() || !frag_shader_create_data.has_value())
        {
            log::error("An OpenGL pipeline requires at least a vertex shader and a fragment shader.");
            return nullptr;
        }

        GLuint vert_shader = compile_shader(vert_shader_create_data->path, GL_VERTEX_SHADER);
        if (!vert_shader)
            return nullptr;

        GLuint frag_shader = compile_shader(frag_shader_create_data->path, GL_FRAGMENT_SHADER);
        if (!frag_shader)
        {
            glDeleteShader(vert_shader);
            return nullptr;
        }

        GLuint program = glCreateProgram();
        glAttachShader(program, vert_shader);
        glAttachShader(program, frag_shader);

        GLuint geom_shader = 0;
        if (use_geometry_shader)
        {
            geom_shader = compile_shader(geom_shader_create_data->path, GL_GEOMETRY_SHADER);
            if (!geom_shader)
            {
                glDeleteShader(vert_shader);
                glDeleteShader(frag_shader);
                glDeleteProgram(program);
                return nullptr;
            }

            glAttachShader(program, geom_shader);
        }

        glLinkProgram(program);

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        if (geom_shader) glDeleteShader(geom_shader);

        // Get linking errors
        GLint success;
        GLchar info_log[512];
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glDeleteProgram(program);

            glGetProgramInfoLog(program, 512, nullptr, info_log);
            if (use_geometry_shader)
            {
                log::error("Failed to link shader program from:\n"
                           " - Vertex shader:   '{}'\n"
                           " - Geometry shader: '{}'\n"
                           " - Fragment shader: '{}'\n"
                           " - Info log: {}",
                    vert_shader_create_data->path, geom_shader_create_data->path, frag_shader_create_data->path, info_log);
            }
            else
            {
                log::error("Failed to link shader program from:\n"
                           " - Vertex shader:   '{}'\n"
                           " - Fragment shader: '{}'\n"
                           " - Info log: {}",
                    vert_shader_create_data->path, frag_shader_create_data->path, info_log);
            }

            return nullptr;
        }

        if (use_geometry_shader)
        {
            log::info("Created OpenGL pipeline from:\n"
                      " - Vertex shader:   '{}'\n"
                      " - Geometry shader: '{}'\n"
                      " - Fragment shader: '{}'",
                vert_shader_create_data->path, geom_shader_create_data->path, frag_shader_create_data->path);
        }
        else
        {
            log::info("Created OpenGL pipeline from:\n"
                      " - Vertex shader:   '{}'\n"
                      " - Fragment shader: '{}'",
                vert_shader_create_data->path, frag_shader_create_data->path);
        }

        return std::shared_ptr<Pipeline_OpenGL>(new Pipeline_OpenGL(program));
    }

    Pipeline_OpenGL::Pipeline_OpenGL(GLuint program)
        : m_program(program)
    {}

    Pipeline_OpenGL::Pipeline_OpenGL(Pipeline_OpenGL&& other) noexcept
    {
        m_program = other.m_program;
        other.m_program = 0;
    }

    Pipeline_OpenGL::~Pipeline_OpenGL()
    {
        if (m_program)
            glDeleteProgram(m_program);
    }

    void Pipeline_OpenGL::set_uniform_mat4(i32 location, const m4& value)
    {
        assert(m_program);
        glProgramUniformMatrix4fv(m_program, location, 1, GL_FALSE, glm::value_ptr(value));
    }
}