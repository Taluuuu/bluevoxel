#include "voxel/chunk_region.h"

#include <core/engine.h>
#include <voxel/voxel_pack.h>

#include "voxel/chunk_column.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    ChunkRegion::ChunkRegion(v2i position)
        : m_position(position)
    {
    }

    void ChunkRegion::place_structures(Chunk& chunk) const
    {
        const auto& voxel_pack = g_engine->get_module_checked<VoxelModule>().voxel_pack();
        const VoxelStructureManager& structure_mgr = voxel_pack->structure_manager();
        for (const auto& [structure_id, structure_min] : m_structures)
        {
            const auto structure = structure_mgr.get_structure(structure_id);
            if (!structure)
                continue;

            const v3i structure_size = structure->size();

            const v3i chunk_min = chunk.chunk_pos() * voxel_constants::chunk_size;

            const v3i min = glm::max(structure_min, chunk_min);
            const v3i max = glm::min(
                structure_min + structure_size,
                chunk_min + v3i{ voxel_constants::chunk_size });

            voxel_utils::for_v3i(min, max,
                [&](const v3i& block_pos)
                {
                    const v3i pos_in_chunk = block_pos - chunk_min;
                    const v3i pos_in_structure = block_pos - structure_min;
                    chunk.set_block_at(pos_in_chunk, structure->get_block(pos_in_structure));
                }
            );
        }
    }
}
