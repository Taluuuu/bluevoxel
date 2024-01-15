#include "rendering/renderer_base.h"

#include "rendering/buffer.h"
#include "rendering/pipeline.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "rendering/vertex_array.h"

#include <glm/gtx/vector_angle.hpp>

namespace h2o::gfx
{
    // Adapted from http://www.songho.ca/opengl/gl_cylinder.html
    static std::vector<v2> calc_unit_circle_vertices(i32 num_sectors)
    {
        const float sector_step = glm::two_pi<f32>() / f32(num_sectors);

        std::vector<v2> unit_circle_vertices{};
        for (i32 i = 0; i <= num_sectors; i++)
        {
            const f32 sector_angle = f32(i) * sector_step;
            unit_circle_vertices.emplace_back(
                cos(sector_angle),
                sin(sector_angle));
        }

        return unit_circle_vertices;
    }

    struct CylinderVerticesAndIndices
    {
        std::vector<v3> vertices{};
        std::vector<u32> indices{};
    };

    static CylinderVerticesAndIndices build_cylinder_vertices(i32 num_sectors)
    {
        CylinderVerticesAndIndices result{};
        auto& vertices = result.vertices;
        auto& indices = result.indices;

        const std::vector<v2> unit_vertices = calc_unit_circle_vertices(num_sectors);

        const f32 height = 1.0f;
        const f32 radius = 0.5f;

        for (i32 i = 0; i < 2; i++)
        {
            const f32 h = f32(i) * height;

            for (i32 j = 0; j <= num_sectors; j++)
            {
                const v2 unit_pos = unit_vertices[j];
                vertices.emplace_back(unit_pos.x * radius, h, unit_pos.y * radius);
            }
        }

        i32 base_center_index = i32(vertices.size());
        i32 top_center_index = base_center_index + num_sectors + 1; // Include center vertex

        for (i32 i = 0; i < 2; i++)
        {
            const f32 h = f32(i) * height;

            // Center point
            vertices.emplace_back(0.0f, h, 0.0f);

            for (i32 j = 0; j < num_sectors; j++)
            {
                const v2 unit_pos = unit_vertices[j];
                vertices.emplace_back(unit_pos.x * radius, h, unit_pos.y * radius);
            }
        }

        // Generate CCW index list of cylinder triangles
        i32 k1 = 0;                         // 1st vertex index at base
        i32 k2 = num_sectors + 1;           // 1st vertex index at top

        // Indices for the side surface
        for (i32 i = 0; i < num_sectors; ++i, ++k1, ++k2)
        {
            // 2 triangles per sector
            // k1 => k1+1 => k2
            indices.push_back(k1);
            indices.push_back(k1 + 1);
            indices.push_back(k2);

            // k2 => k1+1 => k2+1
            indices.push_back(k2);
            indices.push_back(k1 + 1);
            indices.push_back(k2 + 1);
        }

        // Indices for the base surface
        for (i32 i = 0, k = base_center_index + 1; i < num_sectors; ++i, ++k)
        {
            if(i < num_sectors - 1)
            {
                indices.push_back(base_center_index);
                indices.push_back(k + 1);
                indices.push_back(k);
            }
            else // Last triangle
            {
                indices.push_back(base_center_index);
                indices.push_back(base_center_index + 1);
                indices.push_back(k);
            }
        }

        // Indices for the top surface
        for(i32 i = 0, k = top_center_index + 1; i < num_sectors; ++i, ++k)
        {
            if(i < num_sectors - 1)
            {
                indices.push_back(top_center_index);
                indices.push_back(k);
                indices.push_back(k + 1);
            }
            else // Last triangle
            {
                indices.push_back(top_center_index);
                indices.push_back(k);
                indices.push_back(top_center_index + 1);
            }
        }

        return result;
    }

    bool Renderer_Base::init(IWindow& window, const GameInfo& game_info)
    {
        // Setup immediate drawing
        m_colored_line_pipeline = create_pipeline()
            .with_feature(gfx::PipelineFeature::DepthTest)
            .add_shader(gfx::ShaderStage::Vertex, "../Resources/engine/shaders/colored_line.vert")
            .add_shader(gfx::ShaderStage::Fragment, "../Resources/engine/shaders/colored_line.frag")
            .compile();

        if (!m_colored_line_pipeline)
            return false;

        m_colored_shape_pipeline = create_pipeline()
            .with_feature(gfx::PipelineFeature::DepthTest)
            .add_shader(gfx::ShaderStage::Vertex, "../Resources/engine/shaders/colored_shape.vert")
            .add_shader(gfx::ShaderStage::Fragment, "../Resources/engine/shaders/colored_shape.frag")
            .compile();

        if (!m_colored_shape_pipeline)
            return false;

        m_debug_lines_vbo = create_buffer_ptr();
        m_debug_lines_vao = create_vertex_array_ptr();
        m_debug_lines_vao->attach_vertex_buffer(m_debug_lines_vbo, 0, 0, 7 * sizeof(f32));
        m_debug_lines_vao->setup_attribute_float(0, 0, AttributeType::F32, false, 3, 0);
        m_debug_lines_vao->setup_attribute_float(1, 0, AttributeType::F32, false, 4, 3 * sizeof(f32));

        auto [cylinder_vertices, cylinder_indices] = build_cylinder_vertices(16);
        m_cylinder_vertex_count = cylinder_indices.size();
        m_cylinder_vbo = create_buffer_ptr();
        m_cylinder_vbo->update_data(cylinder_vertices.data(), cylinder_vertices.size() * sizeof(v3), BufferUsage::StaticDraw);
        m_cylinder_ebo = create_buffer_ptr();
        m_cylinder_ebo->update_data(cylinder_indices.data(), cylinder_indices.size() * sizeof(u32), BufferUsage::StaticDraw);
        m_cylinder_vao = create_vertex_array_ptr();
        m_cylinder_vao->attach_index_buffer(m_cylinder_ebo);
        m_cylinder_vao->attach_vertex_buffer(m_cylinder_vbo, 0, 0, 3 * sizeof(f32));
        m_cylinder_vao->setup_attribute_float(0, 0, AttributeType::F32, false, 3, 0);

        return true;
    }

    void Renderer_Base::cleanup()
    {
        m_colored_line_pipeline = nullptr;
    }

    void Renderer_Base::start_frame()
    {

    }

    void Renderer_Base::post_render()
    {
        draw_debug_shapes();
    }

    void Renderer_Base::end_frame()
    {

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

        bind_pipeline(m_colored_line_pipeline);
        m_colored_line_pipeline->set_uniform_mat4(0, m_proj_view_matrix);

        draw_arrays(*m_debug_lines_vao, m_lines_to_draw.size() * 2, DrawMode::Lines);

        bind_pipeline(m_colored_shape_pipeline);
        for (const auto& cylinder_data : m_cylinders_to_draw)
        {
            const v3 direction = glm::normalize(cylinder_data.end - cylinder_data.start);
            const f32 length = glm::distance(cylinder_data.end, cylinder_data.start);
            const m4 translation = glm::translate(cylinder_data.start);
            const m4 rotation = glm::mat4_cast(glm::rotation({0.0f, 1.0f, 0.0f}, direction));
            const m4 scale = glm::scale(v3{cylinder_data.radius, length, cylinder_data.radius});
            m4 model = translation * rotation * scale;

            m_colored_shape_pipeline->set_uniform_mat4(0, m_proj_view_matrix);
            m_colored_shape_pipeline->set_uniform_mat4(1, model);
            m_colored_shape_pipeline->set_uniform_vec4(2, cylinder_data.color);
            draw_elements(*m_cylinder_vao, m_cylinder_vertex_count, AttributeType::U32);
        }

        m_lines_to_draw.clear();
        m_cylinders_to_draw.clear();
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

    void Renderer_Base::set_camera(const Camera& camera)
    {
        m_camera = camera;

        m_view_matrix = m_camera.calc_view_matrix();
        m_proj_matrix = m_camera.calc_proj_matrix();
        m_proj_view_matrix = m_proj_matrix * m_view_matrix;
    }

    const Camera& Renderer_Base::camera() const
    {
        return m_camera;
    }

    const m4& Renderer_Base::proj_view_matrix() const
    {
        return m_proj_view_matrix;
    }

    void Renderer_Base::draw_line(const v3& origin, const v3& end, const v4& color)
    {
        m_lines_to_draw.push_back({ origin, end, color });
    }

    void Renderer_Base::draw_cylinder(const v3& origin, const v3& end, f32 radius, const v4& color)
    {
        m_cylinders_to_draw.push_back({ origin, end, radius, color });
    }
}