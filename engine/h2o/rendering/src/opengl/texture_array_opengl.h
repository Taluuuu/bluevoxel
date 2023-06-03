#pragma once

#include "rendering/texture_array.h"
#include "rendering/texture.h"

#include <glad/gl.h>
#include <optional>
#include <vector>

namespace h2o::gfx
{
    class Texture_OpenGL;

    class TextureArray_OpenGL : public ITextureArray
    {
    public:

        TextureArray_OpenGL(size_t array_size);
        TextureArray_OpenGL(const TextureArray_OpenGL&) = delete;
        TextureArray_OpenGL(TextureArray_OpenGL&&) = delete;
        ~TextureArray_OpenGL() override;

        // ITextureArray interface
        void set_texture(i32 index, const std::shared_ptr<ITexture>& texture) override;
        void bind(u32 index) override;

    private:

        GLuint m_handle { 0 };

        std::optional<TextureFormat> m_texture_format;

        std::vector<std::shared_ptr<Texture_OpenGL>> m_textures;

    };
}