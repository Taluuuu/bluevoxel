#pragma once

#include "block_model_selection.h"
#include "gizmo.h"
#include "selection_manager.h"
#include "voxel/uncooked_block_model.h"

#include <glm/gtx/hash.hpp>
#include <unordered_set>

namespace h2o
{
    class InputModule;
    class RenderingModule;
}

namespace bluevoxel
{
    class BlockModelEditor;

    class TriangleSelection : public IBlockModelSelection
    {
    public:

        explicit TriangleSelection(BlockModelEditor& model_editor);
        ~TriangleSelection() override = default;

        bool update(
            h2o::UncookedBlockModel& block_model,
            const h2o::BlockType& block_type,
            const h2o::VoxelPack& voxel_pack) override;

    private:

        struct
        {
            std::optional<h2o::UncookedBlockModel::TriangleHandle> triangle_handle{};
            std::unordered_set<u32> vertex_indices{};
        } m_selection{};

        Gizmo m_gizmo;

        BlockModelEditor* const m_model_editor = nullptr;

        // Event handles
        h2o::EventHandle m_on_clicked_nothing_event_handle{};

        // Module refs
        h2o::InputModule* const m_input_module = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;

    };
}