#pragma once

#include "core/events.h"
#include "rendering/renderer_base.h"

namespace h2o::gfx
{
    class Pipeline_OpenGL;

    class Renderer_OpenGL : public Renderer_Base
    {
    public:

        Renderer_OpenGL() = default;

        // IRenderer interface
        bool init(IWindow& window, const GameInfo& game_info) override;
        void start_frame() override;
        void end_frame() override;
        void set_scissor(v2i scissor_pos, v2i scissor_size) override;
        std::shared_ptr<IPipeline> compile_pipeline(const PipelineCreateData& create_data) override;
        void bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) override;
        void draw_arrays(const VertexArray& vertex_array, u32 vertex_count) override;
        void draw_elements(const VertexArray& vertex_array, u32 vertex_count, AttributeType indices_type, u64 byte_offset = 0) override;
        void draw(const Mesh& mesh) override;

        // Renderer_Base interface
        u32 allocate_vertex_array() override;
        void destroy_vertex_array(u32 vertex_array_id) override;
        void attach_vertex_buffer(u32 vertex_array_id, const Buffer& buffer, u32 binding_index, i64 offset, i32 stride) override;
        void update_index_buffer(u32 vertex_array_id, const Buffer& buffer) override;
        void setup_attribute_float(u32 vertex_array_id, u32 attribute_index, u32 binding_index, AttributeType type, bool normalize, i32 size, u32 relative_offset) override;
        void setup_attribute_int(u32 vertex_array_id, u32 attribute_index, u32 binding_index, AttributeType type, i32 size, u32 relative_offset) override;
        u32 allocate_buffer() override;
        void destroy_buffer(u32 buffer_id) override;
        void update_buffer_data(u32 buffer_id, const void* data, size_t size, BufferUsage buffer_usage) override;
        const void* map_buffer_read_only(u32 buffer_id) override;
        void* map_buffer_write_only(u32 buffer_id) override;
        void* map_buffer_read_write(u32 buffer_id) override;
        void unmap_buffer(u32 buffer_id) override;
        u32 allocate_texture() override;
        void destroy_texture(u32 texture_id) override;
        void bind_texture(u32 texture_id, u32 texture_slot) override;
        void update_texture_data(u32 texture_id, const TextureFormat& format, const void* data) override;
        u32 allocate_texture_array() override;
        void destroy_texture_array(u32 texture_array_id) override;
        void init_texture_array(u32 texture_array_id, u32 array_size, const TextureFormat& texture_format) override;
        void bind_texture_array(u32 texture_array_id, u32 texture_slot) override;
        void attach_texture_to_texture_array(u32 texture_array_id, const Texture& texture, u32 index) override;

    private:

        std::shared_ptr<Pipeline_OpenGL> m_bound_pipeline = nullptr;

        EventHandle m_window_resize_event_handle;

    };
}