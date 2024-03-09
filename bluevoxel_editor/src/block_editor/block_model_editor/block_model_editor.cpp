#include "block_model_editor.h"

#include "block_editor/block_editor_workspace.h"
#include "core/engine.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "triangle_selection.h"
#include "ui/imgui.h"
#include "vertex_selection.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

namespace bluevoxel
{
    using TriangleHandle = h2o::UncookedBlockModel::TriangleHandle;
    using Triangle       = h2o::UncookedBlockModel::Triangle;

    BlockModelEditor::BlockModelEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_workspace(&workspace)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        m_selection = std::make_shared<TriangleSelection>(*this);

        set_tick_phases(h2o::TickPhase::Update);
    }

    BlockEditorWorkspace& BlockModelEditor::workspace() const
    {
        assert(m_workspace);
        return *m_workspace;
    }

    void BlockModelEditor::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return;

        // TODO: Make easy function to get the block type in the workspace
        const auto& block_type = voxel_pack->block_types()[m_workspace->selected_block_id()];
        if (!block_type)
            return;

        auto block_model = voxel_pack->get_uncooked_block_model(block_type->model_id);
        if (!block_model)
            return;

        if (ImGui::Begin("Block Model Editor"))
        {
            if (ImGui::CollapsingHeader("Selection Mode", ImGuiTreeNodeFlags_DefaultOpen))
            {
                {
                    auto triangle_selection = dynamic_cast<const TriangleSelection*>(m_selection.get());
                    if (ImGui::RadioButton("Triangle", triangle_selection != nullptr))
                        m_selection = std::make_shared<TriangleSelection>(*this);
                }

                {
                    auto vertex_selection = dynamic_cast<const VertexSelection*>(m_selection.get());
                    if (ImGui::RadioButton("Vertex", vertex_selection != nullptr))
                        m_selection = std::make_shared<VertexSelection>(*this);
                }
            }
        }

        draw_model_edges(*block_model, m_rendering_module->renderer());

        if (m_selection)
        {
            // Refresh block model if need be
            if (m_selection->update(*block_model, *block_type, *voxel_pack))
                voxel_pack->build_block_model(block_type->model_id);
        }

        ImGui::End();
    }

    void BlockModelEditor::draw_model_edges(const h2o::UncookedBlockModel& block_model, h2o::gfx::IRenderer& renderer) const
    {
        block_model.for_each_triangle(
            [&](const TriangleHandle& triangle_handle, const Triangle& triangle)
            {
                if (triangle.is_hidden)
                    return;

                const v3 p1 { triangle.vertices[0].position };
                const v3 p2 { triangle.vertices[1].position };
                const v3 p3 { triangle.vertices[2].position };

                const h2o::physics::Triangle physics_triangle{
                    p1 / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    p2 / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    p3 / v3{ h2o::voxel_constants::max_coord_value_per_block },
                };

                const v3 camera_front = renderer.camera().front();
                const v3 line_offset = camera_front * -0.005f;
                renderer.draw_line(line_offset + physics_triangle.p1, line_offset + physics_triangle.p2, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
                renderer.draw_line(line_offset + physics_triangle.p2, line_offset + physics_triangle.p3, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
                renderer.draw_line(line_offset + physics_triangle.p3, line_offset + physics_triangle.p1, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
            }
        );
    }
}