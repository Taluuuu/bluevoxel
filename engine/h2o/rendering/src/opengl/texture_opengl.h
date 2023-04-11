#pragma once

#include "rendering/texture.h"
#include "core/types.h"

#include <glad/gl.h>
#include <memory>
#include <string>

namespace h2o::gfx
{
    class Texture_OpenGL : public ITexture
    {
    public:

        static std::shared_ptr<Texture_OpenGL> create(const std::string& path);

        Texture_OpenGL(const Texture_OpenGL&) = delete;
        Texture_OpenGL(Texture_OpenGL&&) = delete;
        ~Texture_OpenGL() override = default;

        // ITexture interface
        void bind(u32 index) const override;

    private:

        Texture_OpenGL(GLuint handle, v2i size, i32 nb_channels);

    private:

        GLuint m_handle{};

        v2i m_size{};
        i32 m_nb_channels{};

    };
}