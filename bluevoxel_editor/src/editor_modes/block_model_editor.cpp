#include "block_model_editor.h"

#include "block_editor_workspace.h"
#include "core/engine.h"
#include "core/utils.h"
#include "input/input_module.h"
#include "physics/ray_intersections.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

#include <imgui.h>

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
                if (auto vertex_selection = get_if<VertexSelection>(&m_selection))
                {
                    vertex_selection->triangle_handle = std::nullopt;
                    vertex_selection->vertex_indices = {};
                }
                else if (auto vertex_position_selection = get_if<VertexPositionSelection>(&m_selection))
                {
                    vertex_position_selection->clear();
                }
            }
        );

        set_tick_phases(h2o::TickPhase::Update);
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

        if (ImGui::Begin("Block Model Editor"))
        {
            if (ImGui::CollapsingHeader("Selection Mode", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::RadioButton("Triangle Vertex", holds_alternative<VertexSelection>(m_selection)))
                    m_selection.emplace<VertexSelection>();

                if (ImGui::RadioButton("Vertex Position", holds_alternative<VertexPositionSelection>(m_selection)))
                    m_selection.emplace<VertexPositionSelection>();
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

        if (auto vertex_selection = get_if<VertexSelection>(&m_selection))
        {
            const auto& [selected_triangle_handle, vertex_indices] = *vertex_selection;
            m_gizmo.set_enabled(!vertex_indices.empty());

            // Allow selecting triangle
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

                            if (auto vertex_selection = get_if<VertexSelection>(&m_selection))
                            {
                                vertex_selection->triangle_handle = triangle_handle;
                                vertex_selection->vertex_indices = {};
                            }
                        }
                    );
                }
            );

            if (selected_triangle_handle)
            {
                for (u32 i = 0; i < 3; i++)
                {
                    const bool is_selected = vertex_indices.contains(i);

                    const VertexHandle vertex_handle{ *selected_triangle_handle, i };
                    if (auto vertex = block_model->get_vertex(vertex_handle))
                    {
                        const v3 vertex_world_pos = vertex->world_pos();

                        const f32 distance_with_camera = glm::length(camera_pos - vertex_world_pos);
                        const f32 vertex_radius_ = vertex_radius * distance_with_camera;

                        // Draw vertex
                        renderer.draw_sphere(
                            vertex_world_pos,
                            vertex_radius_,
                            is_selected ? selected_vertex_color : unselected_vertex_color);

                        // Allow selecting vertex
                        selection_mgr.add(phys::Sphere{ vertex_world_pos, vertex_radius_ },
                            [this, i, vertex_world_pos](const HoverData& hover_data)
                            {
                                if (!hover_data.click_state.pressed_this_frame)
                                    return;

                                if (auto* vertex_selection = std::get_if<VertexSelection>(&m_selection))
                                {
                                    if (m_input_module->key_state(h2o::Key::LeftShift).held)
                                    {
                                        vertex_selection->vertex_indices.insert(i);
                                        m_gizmo.set_position(vertex_world_pos);
                                    }
                                    else
                                    {
                                        vertex_selection->vertex_indices = { i };
                                        m_gizmo.set_position(vertex_world_pos);
                                    }
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
        }
        else if (auto vertex_position_selection = get_if<VertexPositionSelection>(&m_selection))
        {
            auto& selected_positions = *vertex_position_selection;
            m_gizmo.set_enabled(!selected_positions.empty());

            block_model->for_each_vertex(
                [&](const VertexHandle& vertex_handle, Vertex& vertex)
                {
                    const bool is_selected = selected_positions.contains(vertex.position);

                    const v3 vertex_world_pos = vertex.world_pos();

                    const f32 distance_with_camera = glm::length(camera_pos - vertex_world_pos);
                    const f32 vertex_radius_ = vertex_radius * distance_with_camera;

                    // Draw vertex
                    renderer.draw_sphere(
                        vertex_world_pos,
                        vertex_radius_,
                        is_selected ? selected_vertex_color : unselected_vertex_color);

                    // Allow selecting vertex
                    selection_mgr.add(phys::Sphere{ vertex_world_pos, vertex_radius_ },
                        [this, vertex_world_pos, vertex](const HoverData& hover_data)
                        {
                            if (!hover_data.click_state.pressed_this_frame)
                                return;

                            if (auto* selected_positions = std::get_if<VertexPositionSelection>(&m_selection))
                            {
                                if (m_input_module->key_state(h2o::Key::LeftShift).held)
                                {
                                    selected_positions->insert(vertex.position);
                                    m_gizmo.set_position(vertex_world_pos);
                                }
                                else
                                {
                                    *selected_positions = { vertex.position };
                                    m_gizmo.set_position(vertex_world_pos);
                                }
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
                positions_before_translate.reserve(selected_positions.size());

                for (const v3i& selected_position : selected_positions)
                    positions_before_translate.push_back(selected_position);

                selected_positions.clear();
                for (const v3i& position : positions_before_translate)
                {
                    selected_positions.insert(
                        glm::clamp(
                            position + gizmo_delta,
                            0,
                            h2o::voxel_constants::max_coord_value_per_block));
                }
            }
        }
        else
        {
            m_gizmo.set_enabled(false);
        }

        if (gizmo_delta != v3i{})
            voxel_pack->build_block_model(model_id);
    }

    bool BlockModelEditor::edit_vertex(u32 vertex_index, h2o::BlockVertex& vertex)
    {
        v3u p{ vertex.x, vertex.y, vertex.z };
        v2u uv{ vertex.u, vertex.v };

        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(-FLT_MIN);
        bool was_vertex_edited =
            ImGui::SliderInt(fmt::format("##hidden_{}_x", vertex_index).c_str(), (i32*)(&p.x), 0, 16);

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        was_vertex_edited = was_vertex_edited ||
            ImGui::SliderInt(fmt::format("##hidden_{}_y", vertex_index).c_str(), (i32*)(&p.y), 0, 16);

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        was_vertex_edited = was_vertex_edited ||
            ImGui::SliderInt(fmt::format("##hidden_{}_z", vertex_index).c_str(), (i32*)(&p.z), 0, 16);

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        was_vertex_edited = was_vertex_edited ||
            ImGui::SliderInt(fmt::format("##hidden_{}_u", vertex_index).c_str(), (i32*)(&uv.x), 0, 16);

        ImGui::TableNextColumn();
        ImGui::PushItemWidth(-FLT_MIN);
        was_vertex_edited = was_vertex_edited ||
            ImGui::SliderInt(fmt::format("##hidden_{}_v", vertex_index).c_str(), (i32*)(&uv.y), 0, 16);

        vertex.x = p.x;
        vertex.y = p.y;
        vertex.z = p.z;
        vertex.u = uv.x;
        vertex.v = uv.y;

        return was_vertex_edited;
    }

    void BlockModelEditor::create_face(u32 side_index, h2o::BlockModel& block_model)
    {
        if (side_index < 6)
        {
            block_model.occluded_triangles_per_side[side_index].push_back({});
        }
        else
        {
            block_model.unoccluded_triangles.push_back({});
        }
    }

    void BlockModelEditor::create_triangle(u32 side_index, u32 face_index, h2o::BlockModel& block_model)
    {
//        if (side_index < 6)
//        {
//            block_model.occluded_triangles_per_side[side_index][face_index].push_back({});
//            block_model.occluded_triangles_per_side[side_index][face_index].push_back({});
//            block_model.occluded_triangles_per_side[side_index][face_index].push_back({});
//        }
//        else
//        {
//            block_model.unoccluded_triangles[face_index].push_back({});
//            block_model.unoccluded_triangles[face_index].push_back({});
//            block_model.unoccluded_triangles[face_index].push_back({});
//        }
    }

    void BlockModelEditor::draw_model_edges(const h2o::UncookedBlockModel& block_model, h2o::gfx::IRenderer& renderer) const
    {
        block_model.for_each_triangle(
            [&](const TriangleHandle& triangle_handle, const Triangle& triangle)
            {
                const v3i p1 = triangle[0].position;
                const v3i p2 = triangle[1].position;
                const v3i p3 = triangle[2].position;

                const h2o::physics::Triangle physics_triangle{
                    v3{ p1 } / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    v3{ p2 } / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    v3{ p3 } / v3{ h2o::voxel_constants::max_coord_value_per_block },
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