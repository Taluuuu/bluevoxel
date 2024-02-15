#include "block_model_editor.h"

#include "block_editor_workspace.h"
#include "core/engine.h"
#include "input/input_module.h"
#include "physics/math_helpers.h"
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
    BlockModelEditor::BlockModelEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_gizmo(this, workspace.selection_mgr())
        , m_workspace(&workspace)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        m_gizmo.increment_size = 1.0f / h2o::voxel_constants::max_coord_value_per_block;
        m_gizmo.bounds = Gizmo::Bounds{ v3{ 0.0f }, v3{ 1.0f } };

        workspace.selection_mgr().on_clicked_nothing.add_listener(m_on_clicked_nothing_event_handle,
            [this](const SelectionManager::OnClickedNothing&)
            {
//                m_selected_vertices.clear();
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

//        if (ImGui::Begin("Block Model Editor"))
//        {
//            if (ImGui::CollapsingHeader("Selection Mode", ImGuiTreeNodeFlags_DefaultOpen))
//            {
//                if (ImGui::RadioButton("Triangle Vertex", holds_alternative<VertexIndexSelection>(m_selection)))
//                    m_selection.emplace<VertexIndexSelection>();
//
//                if (ImGui::RadioButton("Vertex Position", holds_alternative<VertexPositionSelection>(m_selection)))
//                    m_selection.emplace<VertexPositionSelection>();
//            }
//        }
//        ImGui::End();

//        m_gizmo.set_enabled(!m_selected_vertices.empty());

        auto& renderer = m_rendering_module->renderer();
        auto& selection_mgr = m_workspace->selection_mgr();

        const auto model_id = block_type->model_id;
        h2o::BlockModel block_model = voxel_pack->block_models()[model_id];
        
        u32 side_index = 0;
        for (const auto& side : block_model.occluded_triangles_per_side)
        {
            u32 face_index = 0;
            for (const auto& face : side)
            {
                u32 triangle_index = 0;
                for (const auto& triangle : face)
                {
                    const auto& p1 = triangle[0];
                    const auto& p2 = triangle[1];
                    const auto& p3 = triangle[2];

                    const h2o::physics::Triangle physics_triangle{
                        .p1 = v3{ p1.x, p1.y, p1.z } / v3{ h2o::voxel_constants::max_coord_value_per_block },
                        .p2 = v3{ p2.x, p2.y, p2.z } / v3{ h2o::voxel_constants::max_coord_value_per_block },
                        .p3 = v3{ p3.x, p3.y, p3.z } / v3{ h2o::voxel_constants::max_coord_value_per_block },
                    };

                    const v3& camera_front = renderer.camera().front();
                    const v3 line_offset = camera_front * -0.005f;
                    renderer.draw_line(line_offset + physics_triangle.p1, line_offset + physics_triangle.p2, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
                    renderer.draw_line(line_offset + physics_triangle.p2, line_offset + physics_triangle.p3, v4{ 0.0f, 0.0f, 0.0f, 1.0f });
                    renderer.draw_line(line_offset + physics_triangle.p3, line_offset + physics_triangle.p1, v4{ 0.0f, 0.0f, 0.0f, 1.0f });

//                    // Allow selecting triangle
//                    selection_mgr.add(physics_triangle,
//                        [this, side_index, face_index, triangle_index, physics_triangle]
//                        (const HoverData& hover_data)
//                        {
//                            if (hover_data.click_state.pressed_this_frame)
//                            {
//                                m_selected_side_index = side_index;
//                                m_selected_face_index = face_index;
//                                m_selected_triangle_index = triangle_index;
//                                m_selected_vertex_index.reset();
//
//                                m_gizmo.set_position(physics_triangle.calc_center());
//                            }
//                        }
//                    );

                    for (u32 i = 0; i < 3; i++)
                    {
                        const bool is_selected = is_vertex_selected(side_index, face_index, triangle_index, i, block_model);

                        const v3& vertex_pos_world = physics_triangle[i];

                        // Draw dot
                        renderer.draw_sphere(
                            vertex_pos_world,
                            vertex_radius, is_selected ? selected_vertex_color : unselected_vertex_color);

                        // Allow selecting dot
                        selection_mgr.add(phys::Sphere{ vertex_pos_world, 0.05f },
                            [this, side_index, face_index, triangle_index, triangle, i, vertex_pos_world]
                                (const HoverData& hover_data)
                            {
                                if (!hover_data.click_state.pressed_this_frame)
                                    return;

                                if (auto* pos_selection = std::get_if<VertexPositionSelection>(&m_selection))
                                {
                                    const auto& vertex = triangle[i];
                                    pos_selection->selected_vertex_positions.emplace_back(vertex.x, vertex.y, vertex.z);
                                    m_gizmo.set_position(vertex_pos_world);
                                }
                            }
                        );
                    }

                    triangle_index++;
                }

                face_index++;
            }

            side_index++;
        }

        bool should_refresh_model = false;

//        if (m_selected_side_index)
//        {
//            if (m_selected_face_index)
//            {
//                if (m_selected_triangle_index)
//                {
//                    auto& triangle = block_model.occluded_faces_per_side
//                        [*m_selected_side_index]
//                        [*m_selected_face_index]
//                        [*m_selected_triangle_index];
//
//                    // TODO: Forgor where the 16 is stored
//                    const v3i new_gizmo_pos{ m_gizmo.position() * 16.0f };
//                    const v3i gizmo_delta{ m_gizmo.movement_delta() * 16.0f };
//
//                    if (m_selected_vertex_index)
//                    {
//                        auto& vertex = triangle[*m_selected_vertex_index];
//                        vertex.set_position(new_gizmo_pos);
//                    }
//                    else
//                    {
//                        for (auto& vertex : triangle)
//                        {
//                            vertex.set_position(
//                                glm::clamp(vertex.position() + gizmo_delta, 0, 16));
//                        }
//                    }
//
//                    should_refresh_model = true;
//                }
//            }
//        }

        if (should_refresh_model)
            voxel_pack->edit_block_model(model_id, block_model);
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
        if (side_index < 6)
        {
            block_model.occluded_triangles_per_side[side_index][face_index].push_back({});
            block_model.occluded_triangles_per_side[side_index][face_index].push_back({});
            block_model.occluded_triangles_per_side[side_index][face_index].push_back({});
        }
        else
        {
            block_model.unoccluded_triangles[face_index].push_back({});
            block_model.unoccluded_triangles[face_index].push_back({});
            block_model.unoccluded_triangles[face_index].push_back({});
        }
    }

    bool BlockModelEditor::is_vertex_selected(
        u32 side_index,
        u32 face_index,
        u32 triangle_index,
        u32 vertex_index,
        const h2o::BlockModel& block_model) const
    {
//        if (const auto* index_selection = std::get_if<VertexIndexSelection>(&m_selection))
//        {
//            const auto& indices = index_selection->selected_vertex_indices;
//            const VertexIndexSelection::VertexID index{ side_index, face_index, triangle_index, vertex_index };
//
//            return std::find(indices.begin(), indices.end(), index) != indices.end();
//        }
//        else if (const auto* pos_selection = std::get_if<VertexPositionSelection>(&m_selection))
//        {
//            const auto& positions = pos_selection->selected_vertex_positions;
//            const auto& vertex = block_model.occluded_faces_per_side[side_index][face_index][triangle_index][vertex_index];
//            const v3i pos{ vertex.x, vertex.y, vertex.z };
//
//            return std::find(positions.begin(), positions.end(), pos) != positions.end();
//        }

        return false;
    }
}