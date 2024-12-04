#include "voxel_rendering/voxel_world_renderer.h"

#include "core/engine.h"
#include "rendering/buffer.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "voxel/chunk_manager.h"
#include "voxel/voxel_module.h"

namespace h2o
{
    VoxelWorldRenderer::VoxelWorldRenderer(Tickable& owner, ChunkManager& chunk_manager, ChunkRenderMode chunk_render_mode)
        : Tickable(&owner)
        , m_render_mode(chunk_render_mode)
        , m_chunk_manager(&chunk_manager)
        , m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<VoxelModule>())
    {
        set_tick_phases(TickPhase::Update | TickPhase::PreRender | TickPhase::Render);

        chunk_manager.on_cells_updated.add_listener(m_on_chunk_updated_handle,
            [this](const CellsUpdatedEvent& event)
            {
                for (const v3i& chunk_pos : event.updated_cells)
                    queue_chunk_remesh(chunk_pos);
            }
        );

        chunk_manager.on_cells_deleted.add_listener(m_on_chunk_deleted_handle,
            [this](const CellsDeletedEvent& event)
            {
                for (const v2i chunk_column_pos : event.deleted_cell_columns)
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

    void VoxelWorldRenderer::update(const f32 delta_time)
    {
        const std::unique_lock lock{ m_chunks_pending_remesh_mutex };

        erase_if(m_chunks_pending_remesh,
            [&](const v3i& chunk_pos) -> bool
            {
                if (m_render_mode == ChunkRenderMode::DrawChunksWithAdjacentChunks)
                {
                    static constexpr std::array offsets{
                        v2i{ 0, 0 },
                        v2i{ 1, 0 },
                        v2i{-1, 0 },
                        v2i{ 0, 1 },
                        v2i{ 0,-1 },
                    };

                    for (const v2i offset : offsets)
                    {
                        if (!m_chunk_manager->cell_column_exists(offset + v2i{ chunk_pos.x, chunk_pos.z }))
                            return false;
                    }
                }

                g_engine->thread_pool().queue_job(glm::distance(player_pos, voxel_utils::chunk_to_world_pos(chunk_pos)),
                    [this, chunk_pos]
                    {
                        remesh_chunk_immediate(chunk_pos);
                    }
                );

                return true;
            }
        );
    }

    void VoxelWorldRenderer::pre_render()
    {
        {
            std::unique_lock lock{ m_chunks_pending_remesh_mutex };
            g_engine->debug_infos().update_debug_statistic(
                "voxels", "chunks pending remesh",i32(m_chunks_pending_remesh.size()));
        }

        // Send built chunk meshes to the gpu
        {
            std::unique_lock lock{ m_pending_built_meshes_mutex };

            g_engine->debug_infos().update_debug_statistic(
                "voxels", "chunks remeshed this frame",i32(m_pending_built_meshes.size()));

            for (const auto& chunk_mesh : m_pending_built_meshes)
            {
                if (chunk_mesh.vertex_count() == 0)
                {
                    m_chunk_mesh_pool.free_mesh(chunk_mesh.chunk_pos());
                    continue;
                }

                // Make sure the chunk did not get deleted between the time its mesh was queued
                // for rebuild and now
                if (!m_chunk_manager->cell_exists(chunk_mesh.chunk_pos()))
                    continue;

                auto& mesh_data = m_chunk_mesh_pool.fetch_or_create_mesh(chunk_mesh.chunk_pos());

                mesh_data.vertex_count = chunk_mesh.vertex_count();
                const auto& vertices = chunk_mesh.vertices();
                const auto& vertex_buffer = mesh_data.vertex_array.get_vertex_buffer(0);

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
        // bool is_chunk_empty = true;
        // m_chunk_manager->fetch_chunk(chunk_pos,
        //     [&](const Chunk* chunk)
        //     {
        //         is_chunk_empty = !chunk || chunk->is_empty();
        //     }
        // );
        //
        // if (is_chunk_empty)
        //     return;

        std::unique_lock lock{ m_chunks_pending_remesh_mutex };
        if (!m_chunks_pending_remesh.contains(chunk_pos))
            m_chunks_pending_remesh.insert(chunk_pos);
    }

    void VoxelWorldRenderer::update_chunk_lighting(const ChunkView& chunk_view)
    {
        const v3i chunk_pos = chunk_view.center_cell_pos();
        m_chunk_manager->view_mut<ChunkLighting>(chunk_pos, v3i{1},
            [&](ChunkLightingView& lighting_view)
            {
                if (ChunkLighting* chunk_lighting = lighting_view.get(chunk_pos))
                {
                    chunk_lighting->update_lighting(
                        [&](const v3i& block_pos)
                        {
                            return voxel::get_block_at(chunk_view, block_pos, EViewRelativeTo::ViewCenter).value_or(Block::Air);
                        }
                    );
                }
            }
        );
    }

    void VoxelWorldRenderer::remesh_chunk_immediate(const v3i& chunk_pos)
    {
        m_chunk_manager->view_for_meshing(chunk_pos,
            [&](const ChunkView& view)
            {
                update_chunk_lighting(view);

                m_chunk_manager->view<ChunkLighting>(chunk_pos, v3i{1},
                    [&](const ChunkLightingView& lighting_view)
                    {
                        if (const ChunkLighting* chunk_lighting = lighting_view.get(chunk_pos))
                        {
                            ChunkMesh chunk_mesh(view, *chunk_lighting, *m_voxel_module);

                            const std::unique_lock lock{ m_pending_built_meshes_mutex };
                            m_pending_built_meshes.emplace_back(std::move(chunk_mesh));
                        }
                    }
                );
            }
        );
    }
}
