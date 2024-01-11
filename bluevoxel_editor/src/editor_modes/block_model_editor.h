#pragma once

#include "core/tickable.h"

namespace h2o
{
    class UIModule;
    class VoxelModule;
}

namespace bluevoxel
{
    class BlockEditorWorkspace;

    class BlockModelEditor : public h2o::Tickable
    {
    public:

        explicit BlockModelEditor(BlockEditorWorkspace& workspace);
        ~BlockModelEditor() override = default;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

    private:

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Module refs
        h2o::UIModule* const m_ui_module = nullptr;
        h2o::VoxelModule* const m_voxel_module = nullptr;

    };
}