#pragma once

#include "core/tickable.h"

namespace h2o
{
    struct BlockVertex;
    class IUIRenderer;

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

        bool edit_triangle(h2o::IUIRenderer& ui, u32 triangle_index, h2o::BlockVertex& p0, h2o::BlockVertex& p1, h2o::BlockVertex& p2);
        bool edit_vertex(h2o::IUIRenderer& ui, h2o::BlockVertex& vertex);

        void create_triangle();

    private:

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Module refs
        h2o::UIModule* const m_ui_module = nullptr;
        h2o::VoxelModule* const m_voxel_module = nullptr;

    };
}