#include "triangle_selection.h"

#include "block_editor/block_editor_workspace.h"
#include "block_model_editor.h"
#include "block_model_editor_constants.h"
#include "core/engine.h"
#include "input/input_module.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "voxel/uncooked_block_model.h"

namespace bluevoxel
{
    using TriangleHandle = h2o::UncookedBlockModel::TriangleHandle;
    using Triangle = h2o::UncookedBlockModel::Triangle;
    using VertexHandle = h2o::UncookedBlockModel::VertexHandle;
    using Vertex = h2o::UncookedBlockModel::Vertex;

    TriangleSelection::TriangleSelection(BlockModelEditor& model_editor)
        : m_model_editor(&model_editor)
        , m_gizmo(&model_editor, model_editor.workspace().selection_mgr())
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
    {
        model_editor.workspace().selection_mgr().on_clicked_nothing.add_listener(m_on_clicked_nothing_event_handle,
            [this](const SelectionManager::OnClickedNothing&)
            {
                m_selection.triangle_handle = std::nullopt;
                m_selection.vertex_indices = {};
            }
        );
    }

    bool TriangleSelection::update(
        h2o::UncookedBlockModel& block_model,
        const h2o::BlockType& block_type,
        const h2o::VoxelPack& voxel_pack)
    {
        auto& selection_mgr = m_model_editor->workspace().selection_mgr();

        m_gizmo.set_enabled(!m_selection.vertex_indices.empty());
        const v3i gizmo_delta = m_gizmo.movement_delta();

        auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();
        const v3 camera_pos = camera.position();

        // Allow selecting triangle
        block_model.for_each_triangle(
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

                        m_selection.triangle_handle = triangle_handle;
                        m_selection.vertex_indices = {};
                    }
                );
            }
        );

        if (m_selection.triangle_handle)
        {
            for (u32 i = 0; i < 3; i++)
            {
                const bool is_selected = m_selection.vertex_indices.contains(i);

                const VertexHandle vertex_handle{ *m_selection.triangle_handle, i };
                if (auto vertex = block_model.get_vertex(vertex_handle))
                {
                    const v3 vertex_world_pos = vertex->world_pos();

                    const f32 distance_with_camera = glm::length(camera_pos - vertex_world_pos);
                    const f32 vertex_radius_ = model_editor_constants::vertex_radius * distance_with_camera;

                    // Draw vertex
                    renderer.draw_sphere(
                        vertex_world_pos,
                        vertex_radius_,
                        is_selected ? model_editor_constants::selected_vertex_color : model_editor_constants::unselected_vertex_color);

                    // Allow selecting vertex
                    selection_mgr.add(phys::Sphere{ vertex_world_pos, vertex_radius_ },
                        [this, i, vertex_world_pos](const HoverData& hover_data)
                        {
                            if (!hover_data.click_state.pressed_this_frame)
                                return;

                            if (m_input_module->key_state(h2o::Key::LeftShift).held)
                            {
                                m_selection.vertex_indices.insert(i);
                                m_gizmo.set_position(vertex_world_pos);
                            }
                            else
                            {
                                m_selection.vertex_indices = { i };
                                m_gizmo.set_position(vertex_world_pos);
                            }
                        }
                    );

                    if (is_selected)
                    {
                        vertex->position = glm::clamp(
                            vertex->position + gizmo_delta,
                            0,
                            h2o::voxel_constants::max_coord_value_per_block);
                    }
                }
            }
        }

        return gizmo_delta != v3i{};
    }
}