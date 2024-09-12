#include "voxel_rendering/voxel_world_renderer.h"

#include <rendering/buffer.h>

#include "core/engine.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "voxel/chunk_manager.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    VoxelWorldRenderer::VoxelWorldRenderer(Tickable& owner, ChunkManager& chunk_manager)
        : Tickable(&owner)
        , m_chunk_manager(&chunk_manager)
        , m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<VoxelModule>())
    {
        set_tick_phases(TickPhase::PreRender | TickPhase::Render);

        chunk_manager.on_chunks_updated.add_listener(m_on_chunk_updated_handle,
            [this](const ChunksUpdatedEvent& event)
            {
                for (const v3i& chunk_pos : event.updated_chunks)
                    queue_chunk_remesh(chunk_pos);
            }
        );

        chunk_manager.on_chunks_deleted.add_listener(m_on_chunk_deleted_handle,
            [this](const ChunksDeletedEvent& event)
            {
                for (const v2i chunk_column_pos : event.deleted_chunk_columns)
                {
                    for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
                    {
                        const v3i chunk_pos{ chunk_column_pos.x, i, chunk_column_pos.y };
                        m_chunk_mesh_pool.free_mesh(chunk_pos);

                        {
                            std::unique_lock lock{ m_chunks_pending_remesh_mutex };
                            m_chunks_pending_remesh.erase(chunk_pos);
                        }
                    }
                }
            }
        );
    }

    void VoxelWorldRenderer::pre_render()
    {
        // Send built chunk meshes to the gpu
        {
            std::unique_lock lock{ m_pending_built_meshes_mutex };

            for (const auto& chunk_mesh : m_pending_built_meshes)
            {
                // Make sure the chunk did not get deleted between the time its mesh was queued
                // for rebuild and now
                if (!m_chunk_manager->chunk_exists(chunk_mesh.chunk_pos()))
                    continue;

                auto& mesh_data = m_chunk_mesh_pool.fetch_or_create_mesh(chunk_mesh.chunk_pos());

                mesh_data.vertex_count = chunk_mesh.vertex_count();
                const auto& vertices = chunk_mesh.vertices();
                const auto& vertex_buffer = mesh_data.vertex_array.get_vertex_buffer(0);

                if (!vertex_buffer)
                {
                    // First time init of the VAO here
                    auto& vao = mesh_data.vertex_array;
                    vao.attach_vertex_buffer(m_rendering_module->renderer().create_buffer_ptr(), 0, 0, 3 * sizeof(u32));
                    vao.setup_attribute_int(0, 0, gfx::AttributeType::U32, 1, 0);
                    vao.setup_attribute_int(1, 0, gfx::AttributeType::U32, 1, sizeof(u32));
                    vao.setup_attribute_int(2, 0, gfx::AttributeType::U32, 1, 2 * sizeof(u32));
                }

                vertex_buffer->update_data(vertices.data(), vertices.size() * sizeof(u32), gfx::BufferUsage::StaticDraw);
            }

            m_pending_built_meshes.clear();
        }
    }

    void VoxelWorldRenderer::render()
    {
        const auto& pipeline = m_voxel_module->pipeline();
        const auto& block_textures = m_voxel_module->block_textures();

        auto& renderer = m_rendering_module->renderer();
        renderer.bind_pipeline(pipeline);
        pipeline->set_uniform_mat4(0, renderer.proj_view_matrix());

        block_textures->bind(0);
        pipeline->set_uniform_int(2, 0);
        pipeline->set_uniform_vec3(3, glm::normalize(light_dir));
        pipeline->set_uniform_vec3(4, light_color);
        pipeline->set_uniform_float(5, ambient_strength);

        m_chunk_mesh_pool.for_each_chunk_mesh(
            [&](const ChunkMeshRenderData& mesh_data)
            {
                const auto& [vao, vertex_count, chunk_pos] = mesh_data;
                if (vertex_count > 0)
                {
                    pipeline->set_uniform_ivec3(1, chunk_pos);
                    renderer.draw_arrays(vao, vertex_count, gfx::DrawMode::Triangles);
                }
            }
        );
    }

    void VoxelWorldRenderer::queue_chunk_remesh(const v3i& chunk_pos)
    {
        std::unique_lock lock{ m_chunks_pending_remesh_mutex };
        if (m_chunks_pending_remesh.contains(chunk_pos))
            return;

        m_chunks_pending_remesh.insert(chunk_pos);

        g_engine->thread_pool().queue_job(0.0f,
            [this, chunk_pos]
            {
                {
                    std::unique_lock lock{ m_chunks_pending_remesh_mutex };
                    m_chunks_pending_remesh.erase(chunk_pos);
                }

                remesh_chunk_immediate(chunk_pos);
            }
        );
    }

    void VoxelWorldRenderer::remesh_chunk_immediate(const v3i& chunk_pos)
    {
        m_chunk_manager->view<v3u{3}>(chunk_pos - v3i{1},
            [&](const ChunkView<v3u{3}>& view)
            {
                ChunkMesh chunk_mesh(view, *m_voxel_module);
                if (chunk_mesh.vertex_count() > 0)
                {
                    std::unique_lock lock{ m_pending_built_meshes_mutex };
                    m_pending_built_meshes.emplace_back(std::move(chunk_mesh));
                }
            }
        );
    }
}
