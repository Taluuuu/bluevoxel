#pragma once

#include "block_model_selections.h"
#include "core/events.h"
#include "core/tickable.h"
#include "gizmo.h"
#include "uv_editor.h"
#include "voxel/block_model.h"

#include "glm/gtx/hash.hpp"
#include <unordered_set>
#include <variant>

namespace h2o
{
    struct BlockModel;
    struct BlockVertex;

    namespace gfx
    {
        class IRenderer;
    }

    class InputModule;
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
        static constexpr f32 vertex_radius = 0.015f;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        void draw_model_edges(const h2o::UncookedBlockModel& block_model, h2o::gfx::IRenderer& renderer) const;

    private:

        Gizmo m_gizmo;

        SelectionTypes m_selection{};

        UVEditor m_uv_editor{};

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Event handles
        h2o::EventHandle m_on_clicked_nothing_event_handle{};

        // Module refs
        h2o::InputModule*     const m_input_module     = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::VoxelModule*     const m_voxel_module     = nullptr;

    };
}