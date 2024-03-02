#pragma once

#include "block_model_selection.h"
#include "gizmo.h"
#include "selection_manager.h"

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

    class VertexSelection : public IBlockModelSelection
    {
    public:

        explicit VertexSelection(BlockModelEditor& model_editor);
        ~VertexSelection() override = default;

        bool update(
            h2o::UncookedBlockModel& block_model,
            const h2o::BlockType& block_type,
            const h2o::VoxelPack& voxel_pack) override;

    private:

        std::unordered_set<v3i> m_selections{};

        Gizmo m_gizmo;

        BlockModelEditor* const m_model_editor = nullptr;

        // Event handles
        h2o::EventHandle m_on_clicked_nothing_event_handle{};

        // Module refs
        h2o::InputModule* const m_input_module = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;

    };
}