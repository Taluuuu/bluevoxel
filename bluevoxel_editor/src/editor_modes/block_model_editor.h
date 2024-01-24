#pragma once

#include "core/tickable.h"
#include "gizmo.h"

#include <variant>

namespace h2o
{
    struct BlockModel;
    struct BlockVertex;

    class UIModule;
    class RenderingModule;
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

        static bool edit_vertex(u32 vertex_index, h2o::BlockVertex& vertex);

        static void create_face(u32 side_index, h2o::BlockModel& block_model);
        static void create_triangle(u32 side_index, u32 face_index, h2o::BlockModel& block_model);

    private:

        Gizmo m_gizmo;

        struct FaceIndexSelection{ u32 side = 0; u32 face = 0; };
        struct TriangleIndexSelection{ FaceIndexSelection face{}; u32 triangle = 0; };
        struct VertexIndexSelection{ TriangleIndexSelection triangle{}; u32 vertex = 0; };
        struct VertexPositionSelection{ v3i vertex_pos{}; };

        std::variant<std::monostate,
            FaceIndexSelection,
            TriangleIndexSelection,
            VertexIndexSelection,
            VertexPositionSelection> m_selection{};

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Module refs
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::VoxelModule*     const m_voxel_module     = nullptr;

    };
}