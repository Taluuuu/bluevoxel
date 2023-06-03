#pragma once

#include "rendering/texture.h"
#include "core/resources.h"
#include "core/types.h"

#include <glad/gl.h>
#include <memory>
#include <string>

namespace h2o::gfx
{
    class Texture_OpenGL
        : public ITexture
        , public IResource
    {
    public:

        Texture_OpenGL() = default;
        Texture_OpenGL(const Texture_OpenGL&) = delete;
        Texture_OpenGL(Texture_OpenGL&&) = delete;
        ~Texture_OpenGL() override;

        // ITexture interface
        void bind(u32 index) const override;
        [[nodiscard]] const TextureFormat& format() const override;

        // IResource interface
        bool load(const std::string& path) override;

        [[nodiscard]] GLuint handle() const { return m_handle; }

    private:

        GLuint m_handle { 0 };

        TextureFormat m_texture_format{};

    };
}