#include "rendering/renderer_base.h"

#include "rendering/buffer.h"
#include "rendering/pipeline.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "rendering/vertex_array.h"

namespace h2o::gfx
{
    bool Renderer_Base::init(IWindow& window, const GameInfo& game_info)
    {
        // Setup debug drawing
        m_debug_draw_pipeline = create_pipeline()
            .with_feature(gfx::PipelineFeature::DepthTest)
            .add_shader(gfx::ShaderStage::Vertex, "../Resources/engine/shaders/debug_shape.vert")
            .add_shader(gfx::ShaderStage::Fragment, "../Resources/engine/shaders/debug_shape.frag")
            .compile();

        if (!m_debug_draw_pipeline)
            return false;

        m_debug_lines_vbo = create_buffer_ptr();

        m_debug_lines_vao = create_vertex_array_ptr();
        m_debug_lines_vao->attach_vertex_buffer(m_debug_lines_vbo, 0, 0, 7 * sizeof(f32));
        m_debug_lines_vao->setup_attribute_float(0, 0, AttributeType::F32, false, 3, 0);
        m_debug_lines_vao->setup_attribute_float(1, 0, AttributeType::F32, false, 4, 3 * sizeof(f32));

        return true;
    }

    void Renderer_Base::cleanup()
    {
        m_debug_draw_pipeline = nullptr;
    }

    void Renderer_Base::start_frame()
    {

    }

    void Renderer_Base::end_frame()
    {
        draw_debug_shapes();
    }

    void Renderer_Base::draw_debug_shapes()
    {
        // TODO: Store the buffer directly instead of allocating this on the heap every frame
        std::vector<f32> buffer{};
        buffer.reserve(m_lines_to_draw.size() * 7 * 2);
        for (const auto& line : m_lines_to_draw)
        {
            // P1
            buffer.push_back(line.start.x);
            buffer.push_back(line.start.y);
            buffer.push_back(line.start.z);
            buffer.push_back(line.color.x);
            buffer.push_back(line.color.y);
            buffer.push_back(line.color.z);
            buffer.push_back(line.color.w);

            // P2
            buffer.push_back(line.end.x);
            buffer.push_back(line.end.y);
            buffer.push_back(line.end.z);
            buffer.push_back(line.color.x);
            buffer.push_back(line.color.y);
            buffer.push_back(line.color.z);
            buffer.push_back(line.color.w);
        }

        // TODO: There should be a preallocated buffer instead of calling update data every frame
        m_debug_lines_vbo->update_data(buffer.data(), buffer.size() * sizeof(f32), BufferUsage::DynamicDraw);

        bind_pipeline(m_debug_draw_pipeline);
        m_debug_draw_pipeline->set_uniform_mat4(0, m_proj_view_matrix);

        draw_arrays(*m_debug_lines_vao, m_lines_to_draw.size() * 2, DrawMode::Lines);

        m_lines_to_draw.clear();
    }

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

    void Renderer_Base::set_proj_view_matrix(const m4& proj_view)
    {
        m_proj_view_matrix = proj_view;
    }

    void Renderer_Base::draw_debug_line(const v3& origin, const v3& end, const v4& color)
    {
        m_lines_to_draw.push_back({ origin, end, color });
    }
}