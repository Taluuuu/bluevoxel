#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "gizmo.h"
#include "voxel/block_model.h"

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

    public:

        static constexpr v4 unselected_vertex_color{ 0.1f, 0.1f, 0.1f, 1.0f };
        static constexpr v4 selected_vertex_color{ 1.0f, 0.9f, 0.2f, 1.0f };
        static constexpr f32 vertex_radius = 0.05f;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        static bool edit_vertex(u32 vertex_index, h2o::BlockVertex& vertex);

        static void create_face(u32 side_index, h2o::BlockModel& block_model);
        static void create_triangle(u32 side_index, u32 face_index, h2o::BlockModel& block_model);

        [[nodiscard]] bool is_vertex_selected(
            u32 side_index, u32 face_index, u32 triangle_index, u32 vertex_index,
            const h2o::BlockModel& block_model) const;

    private:

        Gizmo m_gizmo;

//        struct VertexIndexSelection
//        {
//            std::vector<h2o::BlockModel::VertexID> selected_vertex_indices{};
//        };
//
//        struct VertexPositionSelection
//        {
//            std::vector<v3i> selected_vertex_positions{};
//        };
//
//        std::variant<
//            VertexIndexSelection,
//            VertexPositionSelection> m_selection{};

//        std::vector<VertexIndexSelection> m_selected_vertices{};

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Event handles
        h2o::EventHandle m_on_clicked_nothing_event_handle{};

        // Module refs
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::VoxelModule*     const m_voxel_module     = nullptr;

    };
}