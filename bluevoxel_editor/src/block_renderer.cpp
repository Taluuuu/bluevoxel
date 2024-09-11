#include "block_renderer.h"

#include "core/engine.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/texture_array.h"
#include "voxel/chunk.h"
#include "voxel/chunk_view.h"
#include "voxel/voxel_bounds.h"
#include "voxel/voxel_module.h"
#include "voxel_rendering/chunk_mesh_pool.h"

namespace bluevoxel
{
    BlockRenderer::BlockRenderer()
        : m_voxel_rendering_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        auto& voxel_module = g_engine->get_module_checked<h2o::VoxelModule>();

        m_chunk = std::make_shared<h2o::Chunk>();
        m_chunk->init(v3i{ 0, 0, 0 }, voxel_module);

        m_chunk_view = std::make_shared<h2o::ChunkView>(v3i{ 0, 0, 0 }, v3i{ 1, 1, 1 });
        m_voxel_bounds = std::make_shared<h2o::VoxelBounds>(v2i{ 0, 0 }, 0);
        m_chunk_mesh_pool = std::make_shared<h2o::ChunkMeshPool>();

        // TODO: Use a chunk manager here and remove public access to add_chunk
        m_chunk_view->add_chunk(*m_chunk);
    }

    void BlockRenderer::set_block(h2o::Block block)
    {
        m_chunk->set_block_at({ 0, 0, 0 }, block);

        refresh();
    }

    void BlockRenderer::refresh()
    {
        m_chunk_mesh_pool->build_chunk_mesh(*m_chunk_view);
        m_chunk_mesh_pool->update_meshes(*m_voxel_bounds);
    }

    void BlockRenderer::render(h2o::gfx::IRenderer& renderer)
    {
        const auto& block_textures = m_voxel_rendering_module->block_textures();
        if (!block_textures)
            return;

        block_textures->bind(0);

        const auto& pipeline = m_voxel_rendering_module->pipeline();

        renderer.bind_pipeline(pipeline);
        pipeline->set_uniform_mat4(0, renderer.proj_view_matrix());
        pipeline->set_uniform_int(2, 0);
        pipeline->set_uniform_vec3(3, glm::normalize(light_dir));
        pipeline->set_uniform_vec3(4, light_color);
        pipeline->set_uniform_float(5, ambient_strength);

        m_chunk_mesh_pool->for_each_chunk_mesh(
            [&](const h2o::ChunkMeshRenderData& chunk_mesh)
            {
                if (chunk_mesh.vertex_count > 0)
                {
                    pipeline->set_uniform_ivec3(1, chunk_mesh.chunk_pos);
                    renderer.draw_arrays(chunk_mesh.vertex_array, chunk_mesh.vertex_count, h2o::gfx::DrawMode::Triangles);
                }
            }
        );
    }
}