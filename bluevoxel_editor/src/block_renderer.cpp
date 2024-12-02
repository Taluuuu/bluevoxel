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
    BlockRenderer::BlockRenderer(h2o::Tickable& owner)
        : m_voxel_world_renderer(owner, m_chunk_manager, h2o::ChunkRenderMode::DrawAllChunks)
    {}

    void BlockRenderer::set_block(const h2o::Block block)
    {
        m_chunk_manager.fetch_mut<h2o::Chunk>(v3i{0},
            [&](h2o::Chunk* chunk)
            {
                if (chunk)
                {
                    chunk->set_block_at(v3i{0}, block);
                    chunk->mark_generated();
                }
            }, true
        );

        m_chunk_manager.broadcast_events();
    }
}