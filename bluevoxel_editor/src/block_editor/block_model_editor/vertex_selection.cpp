#include "vertex_selection.h"

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
    using VertexHandle = h2o::UncookedBlockModel::VertexHandle;
    using Vertex = h2o::UncookedBlockModel::Vertex;

    VertexSelection::VertexSelection(BlockModelEditor& model_editor)
        : m_model_editor(&model_editor)
        , m_gizmo(&model_editor, model_editor.workspace().selection_mgr())
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
    {
        model_editor.workspace().selection_mgr().on_clicked_nothing.add_listener(m_on_clicked_nothing_event_handle,
            [this](const SelectionManager::OnClickedNothing&)
            {
                m_selections.clear();
            }
        );

        m_gizmo.increment_size = 1.0f / h2o::voxel_constants::max_coord_value_per_block;
        m_gizmo.bounds = Gizmo::Bounds{ v3{ 0.0f }, v3{ 1.0f } };
    }

    bool VertexSelection::update(
        h2o::UncookedBlockModel& block_model,
        const h2o::BlockType& block_type,
        const h2o::VoxelPack& voxel_pack)
    {
        auto& selection_mgr = m_model_editor->workspace().selection_mgr();

        m_gizmo.set_enabled(!m_selections.empty());
        const v3i gizmo_delta{ m_gizmo.movement_delta() * f32(h2o::voxel_constants::max_coord_value_per_block) };

        auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();
        const v3 camera_pos = camera.position();

        block_model.for_each_vertex(
            [&](const VertexHandle& vertex_handle, Vertex& vertex)
            {
                const bool is_selected = m_selections.contains(vertex.position);

                const v3 vertex_world_pos = vertex.world_pos();

                const f32 distance_with_camera = glm::length(camera_pos - vertex_world_pos);
                const f32 vertex_radius = model_editor_constants::vertex_radius * distance_with_camera;

                // Draw vertex
                renderer.draw_sphere(
                    vertex_world_pos,
                    vertex_radius,
                    is_selected ? model_editor_constants::selected_vertex_color : model_editor_constants::unselected_vertex_color);

                // Allow selecting vertex
                selection_mgr.add(phys::Sphere{ vertex_world_pos, vertex_radius },
                    [this, vertex_world_pos, vertex](const HoverData& hover_data)
                    {
                        if (!hover_data.click_state.pressed_this_frame)
                            return;

                        if (m_input_module->key_state(h2o::Key::LeftShift).held)
                        {
                            m_selections.insert(vertex.position);
                            m_gizmo.set_position(vertex_world_pos);
                        }
                        else
                        {
                            m_selections = { vertex.position };
                            m_gizmo.set_position(vertex_world_pos);
                        }
                    }
                );

                if (is_selected)
                {
                    vertex.position = glm::clamp(
                        vertex.position + gizmo_delta,
                        0,
                        h2o::voxel_constants::max_coord_value_per_block);
                }
            }
        );

        if (gizmo_delta != v3i{})
        {
            // Translate all selected positions
            std::vector<v3i> positions_before_translate{};
            positions_before_translate.reserve(m_selections.size());

            for (const v3i& selected_position : m_selections)
                positions_before_translate.push_back(selected_position);

            m_selections.clear();
            for (const v3i& position : positions_before_translate)
            {
                m_selections.insert(
                    glm::clamp(
                        position + gizmo_delta,
                        0,
                        h2o::voxel_constants::max_coord_value_per_block));
            }
        }

        return gizmo_delta != v3i{};
    }
}