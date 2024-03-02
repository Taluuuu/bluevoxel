#include "block_model_editor.h"

#include "block_editor/block_editor_workspace.h"
#include "block_model_editor_constants.h"
#include "core/engine.h"
#include "core/utils.h"
#include "input/input_module.h"
#include "physics/ray_intersections.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "ui/imgui.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

namespace bluevoxel
{
    using TriangleHandle = h2o::UncookedBlockModel::TriangleHandle;
    using Triangle = h2o::UncookedBlockModel::Triangle;
    using VertexHandle = h2o::UncookedBlockModel::VertexHandle;
    using Vertex = h2o::UncookedBlockModel::Vertex;

    BlockModelEditor::BlockModelEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_gizmo(this, workspace.selection_mgr())
        , m_workspace(&workspace)
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        m_gizmo.increment_size = 1.0f / h2o::voxel_constants::max_coord_value_per_block;
        m_gizmo.bounds = Gizmo::Bounds{ v3{ 0.0f }, v3{ 1.0f } };

        workspace.selection_mgr().on_clicked_nothing.add_listener(m_on_clicked_nothing_event_handle,
            [this](const SelectionManager::OnClickedNothing&)
            {
                if (auto face_selection = get_if<FaceSelection>(&m_selection))
                {
                    *face_selection = std::nullopt;
                }
            }
        );

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

        const auto model_id = block_type->model_id;
        auto block_model = voxel_pack->get_uncooked_block_model(model_id);
        if (!block_model)
            return;

        bool should_refresh_model = false;

        if (ImGui::Begin("Block Model Editor"))
        {
            if (ImGui::CollapsingHeader("Selection Mode", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::RadioButton("Face", holds_alternative<FaceSelection>(m_selection)))
                    m_selection.emplace<FaceSelection>();

//                if (ImGui::RadioButton("Triangle", holds_alternative<TriangleSelection>(m_selection)))
//                    m_selection.emplace<TriangleSelection>();

//                if (ImGui::RadioButton("Vertex", holds_alternative<VertexSelection>(m_selection)))
//                    m_selection.emplace<VertexSelection>();


//                if (const auto triangle_selection = std::get_if<TriangleSelection>(&m_selection))
//                {
//                    should_refresh_model = should_refresh_model ||
//                        m_uv_editor.update(
//                            *block_model,
//                            *voxel_pack,
//                            *block_type,
//                            *triangle_selection);
//                }
//                else
//                {
//                    ImGui::Text("UV editor is only available in triangle selection mode.");
//                }
            }
        }
        ImGui::End();

        auto& renderer = m_rendering_module->renderer();
        auto& selection_mgr = m_workspace->selection_mgr();

        const auto& camera = renderer.camera();
        const v3 camera_pos = camera.position();
        const v3 camera_front = camera.front();

        const v3i new_gizmo_pos{ m_gizmo.position() * f32(h2o::voxel_constants::max_coord_value_per_block) };
        const v3i gizmo_delta{ m_gizmo.movement_delta() * f32(h2o::voxel_constants::max_coord_value_per_block) };

        // Draw edges
        draw_model_edges(*block_model, renderer);

        if (auto face_selection = get_if<FaceSelection>(&m_selection))
        {
            // Allow selecting faces
            block_model->for_each_triangle(
                [&](const TriangleHandle& triangle_handle, Triangle& triangle)
                {
                    const phys::Triangle physics_triangle{
                        triangle[0].world_pos(), triangle[1].world_pos(), triangle[2].world_pos(),
                    };

                    selection_mgr.add(physics_triangle,
                        [this, triangle_handle](const HoverData& hover_data)
                        {
                            if (!hover_data.click_state.pressed_this_frame)
                                return;

                            if (auto face_selection = get_if<FaceSelection>(&m_selection))
                                *face_selection = triangle_handle.face_handle;
                        }
                    );
                }
            );

            if (*face_selection)
            {
//                h2o::log::info("SELECTED");
            }
        }
        else
        {
            m_gizmo.set_enabled(false);
        }

        should_refresh_model = should_refresh_model || gizmo_delta != v3i{};

        if (should_refresh_model)
            voxel_pack->build_block_model(model_id);
    }

    void BlockModelEditor::draw_model_edges(const h2o::UncookedBlockModel& block_model, h2o::gfx::IRenderer& renderer) const
    {
        block_model.for_each_triangle(
            [&](const TriangleHandle& triangle_handle, const Triangle& triangle)
            {
                const v3 p1 { triangle[0].position };
                const v3 p2 { triangle[1].position };
                const v3 p3 { triangle[2].position };

                const h2o::physics::Triangle physics_triangle{
                    p1 / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    p2 / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    p3 / v3{ h2o::voxel_constants::max_coord_value_per_block },
                };

                const v3& camera_front = renderer.camera().front();
                const v3 line_offset = camera_front * -0.005f;
                renderer.draw_line(line_offset + physics_triangle.p1, line_offset + physics_triangle.p2, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
                renderer.draw_line(line_offset + physics_triangle.p2, line_offset + physics_triangle.p3, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
                renderer.draw_line(line_offset + physics_triangle.p3, line_offset + physics_triangle.p1, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
            }
        );
    }
}