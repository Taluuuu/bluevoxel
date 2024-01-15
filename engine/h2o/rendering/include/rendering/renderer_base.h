#pragma once

#include "camera.h"
#include "core/types.h"
#include "renderer_enums.h"
#include "rendering/renderer.h"

#include <vector>

namespace h2o::gfx
{
    class VertexArray;
    class Buffer;
    class Texture;
    struct TextureFormat;

    // This class is meant to house all backend-dependant code and provide lower level access to the graphics api
    class Renderer_Base : public IRenderer
    {
    public:

        Renderer_Base() = default;
        ~Renderer_Base() override = default;

        virtual bool init(IWindow& window, const GameInfo& game_info);
        virtual void cleanup();
        virtual void start_frame();
        virtual void post_render();
        virtual void end_frame();

        void draw_debug_shapes();

        // Pipeline
        virtual std::shared_ptr<IPipeline> compile_pipeline(const PipelineCreateData& create_data) = 0;

        // Vertex Array
        [[nodiscard]] virtual u32 allocate_vertex_array() = 0;
        virtual void destroy_vertex_array(u32 vertex_array_id) = 0;
        virtual void attach_vertex_buffer(u32 vertex_array_id, const Buffer& buffer, u32 binding_index, i64 offset, i32 stride) = 0;
        virtual void update_index_buffer(u32 vertex_array_id, const Buffer& buffer) = 0;
        virtual void setup_attribute_float(u32 vertex_array_id, u32 attribute_index, u32 binding_index, AttributeType type, bool normalize, i32 size, u32 relative_offset) = 0;
        virtual void setup_attribute_int(u32 vertex_array_id, u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset) = 0;

        // Buffer
        [[nodiscard]] virtual u32 allocate_buffer() = 0;
        virtual void destroy_buffer(u32 buffer_id) = 0;
        virtual void update_buffer_data(u32 buffer_id, const void* data, size_t size, BufferUsage buffer_usage) = 0;
        virtual const void* map_buffer_read_only(u32 buffer_id) = 0;
        virtual void* map_buffer_write_only(u32 buffer_id) = 0;
        virtual void* map_buffer_read_write(u32 buffer_id) = 0;
        virtual void unmap_buffer(u32 buffer_id) = 0;

        // Texture
        [[nodiscard]] virtual u32 allocate_texture() = 0;
        virtual void destroy_texture(u32 texture_id) = 0;
        virtual void bind_texture(u32 texture_id, u32 texture_slot) = 0;
        virtual void update_texture_data(u32 texture_id, const TextureFormat& format, const void* data) = 0;

        // Texture Array
        [[nodiscard]] virtual u32 allocate_texture_array() = 0;
        virtual void destroy_texture_array(u32 texture_array_id) = 0;
        virtual void init_texture_array(u32 texture_array_id, u32 array_size, const TextureFormat& texture_format) = 0;
        virtual void bind_texture_array(u32 texture_array_id, u32 texture_slot) = 0;
        virtual void attach_texture_to_texture_array(u32 texture_array_id, const Texture& texture, u32 index) = 0;

        // IRenderer interface
        PipelineCreateData create_pipeline() override;
        Buffer create_buffer() override;
        std::shared_ptr<Buffer> create_buffer_ptr() override;
        VertexArray create_vertex_array() override;
        std::shared_ptr<VertexArray> create_vertex_array_ptr() override;
        Texture create_texture() override;
        std::shared_ptr<Texture> create_texture_ptr() override;
        TextureArray create_texture_array(u32 array_size) override;
        std::shared_ptr<TextureArray> create_texture_array_ptr(u32 array_size) override;
        void set_camera(const Camera& camera) override;
        const Camera& camera() const override;
        const m4& proj_view_matrix() const override;
        void draw_line(const v3& origin, const v3& end, const v4& color) override;
        void draw_cylinder(const v3& origin, const v3& end, f32 radius, const v4& color) override;

    private:

        // Line drawing
        std::shared_ptr<gfx::IPipeline> m_colored_line_pipeline = nullptr;
        std::shared_ptr<VertexArray> m_debug_lines_vao = nullptr;
        std::shared_ptr<Buffer> m_debug_lines_vbo = nullptr;

        struct LineData
        {
            v3 start{}, end{};
            v4 color{};
        };

        std::vector<LineData> m_lines_to_draw{};

        // Cylinder drawing
        std::shared_ptr<gfx::IPipeline> m_colored_shape_pipeline = nullptr;
        std::shared_ptr<VertexArray> m_cylinder_vao = nullptr;
        std::shared_ptr<Buffer> m_cylinder_vbo = nullptr;
        std::shared_ptr<Buffer> m_cylinder_ebo = nullptr;
        u32 m_cylinder_vertex_count = 0;

        struct CylinderData
        {
            v3 start{}, end{};
            f32 radius = 0.0f;
            v4 color{};
        };

        std::vector<CylinderData> m_cylinders_to_draw{};

        Camera m_camera{};
        // Cached camera matrices
        m4 m_view_matrix, m_proj_matrix, m_proj_view_matrix{};

    };
}