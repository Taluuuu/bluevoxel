#pragma once

#include "block_model_selections.h"

#include <optional>

namespace h2o
{
    class BlockType;
    class UncookedBlockModel;
    class VoxelPack;

    class InputModule;
}

namespace bluevoxel
{
    class UVEditor
    {
    public:

        UVEditor();

        // Returns true if the model needs to be refreshed (an UV has been changed)
        bool update(
            h2o::UncookedBlockModel& block_model,
            const h2o::VoxelPack& voxel_pack,
            const h2o::BlockType& block_type);

    private:

        static constexpr f32 point_radius = 10.0f;
        static constexpr v2 texture_offset{ 15.0f, 15.0f };

        f32 m_texture_size = 300.0f;

        std::optional<u32> m_selected_vertex_index{};

        h2o::InputModule* const m_input_module = nullptr;

    };
}