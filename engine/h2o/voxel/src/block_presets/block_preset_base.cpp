#include "voxel/block_presets/block_preset_base.h"

namespace h2o
{
    void BlockPreset_Base::tick(Block& block, Chunk& chunk, const v3i& local_block_pos) const
    {
        // This shouldn't run if should_tick returns true.
        assert(false);
    }
}