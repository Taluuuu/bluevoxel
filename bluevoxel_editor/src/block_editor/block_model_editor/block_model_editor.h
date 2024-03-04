#pragma once

#include "block_model_selections.h"
#include "core/events.h"
#include "core/tickable.h"

#include <glm/gtx/hash.hpp>
#include <unordered_set>
#include <variant>

namespace h2o
{
    struct BlockModel;
    struct BlockVertex;

    class RenderingModule;
    class VoxelModule;

    namespace gfx
    {
        class IRenderer;
    }
}

namespace bluevoxel
{
    class BlockEditorWorkspace;
    class IBlockModelSelection;

    class BlockModelEditor : public h2o::Tickable
    {
    public:

        explicit BlockModelEditor(BlockEditorWorkspace& workspace);
        ~BlockModelEditor() override = default;

        [[nodiscard]] BlockEditorWorkspace& workspace() const;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        void draw_model_edges(const h2o::UncookedBlockModel& block_model, h2o::gfx::IRenderer& renderer) const;

    private:

        std::shared_ptr<IBlockModelSelection> m_selection{};

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Event handles
        h2o::EventHandle m_on_clicked_nothing_event_handle{};

        // Module refs
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::VoxelModule*     const m_voxel_module     = nullptr;

    };
}