#include "rendering/renderer_base.h"

#include "rendering/buffer.h"
#include "rendering/pipeline.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "rendering/vertex_array.h"

namespace h2o::gfx
{
    PipelineCreateData Renderer_Base::create_pipeline()
    {
        return PipelineCreateData(*this);
    }

    Buffer Renderer_Base::create_buffer()
    {
        return Buffer(*this);
    }

    std::shared_ptr<Buffer> Renderer_Base::create_buffer_ptr()
    {
        return std::make_shared<Buffer>(*this);
    }

    VertexArray Renderer_Base::create_vertex_array()
    {
        return VertexArray(*this);
    }

    std::shared_ptr<VertexArray> Renderer_Base::create_vertex_array_ptr()
    {
        return std::make_shared<VertexArray>(*this);
    }

    Texture Renderer_Base::create_texture()
    {
        return Texture();
    }

    std::shared_ptr<Texture> Renderer_Base::create_texture_ptr()
    {
        return std::make_shared<Texture>();
    }

    TextureArray Renderer_Base::create_texture_array(u32 array_size)
    {
        return TextureArray(*this, array_size);
    }

    std::shared_ptr<TextureArray> Renderer_Base::create_texture_array_ptr(u32 array_size)
    {
        return std::make_shared<TextureArray>(*this, array_size);
    }
}