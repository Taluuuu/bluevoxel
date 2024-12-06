#include "block_renderer.h"

#include "core/engine.h"
#include "rendering/pipeline.h"
#include "voxel/chunk.h"
#include "voxel/voxel_bounds.h"
#include "voxel/voxel_module.h"

namespace bluevoxel
{
    BlockRenderer::BlockRenderer(h2o::Tickable& owner)
        : m_voxel_world_renderer(owner, m_chunk_manager, h2o::ChunkRenderMode::DrawAllChunks)
    {}

    void BlockRenderer::set_block(const h2o::Block block)
    {
        m_chunk_manager.fetch<h2o::Chunk>(v3i{0},
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