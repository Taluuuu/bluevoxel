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
        m_gizmo.increment_size = 1.0f / 16;
        m_gizmo.bounds = Gizmo::Bounds{ v3{ 0.0f }, v3{ 1.0f } };

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

        m_gizmo.set_enabled(!holds_alternative<std::monostate>(m_selection));

        auto& renderer = m_rendering_module->renderer();
        auto& selection_mgr = m_workspace->selection_mgr();

        const auto model_id = block_type->model_id;
        h2o::BlockModel block_model = voxel_pack->block_models()[model_id];

        u32 side_index = 0;
        for (const auto& side : block_model.occluded_faces_per_side)
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
                        .p1 = v3{ p1.x, p1.y, p1.z } / v3{ 16 },
                        .p2 = v3{ p2.x, p2.y, p2.z } / v3{ 16 },
                        .p3 = v3{ p3.x, p3.y, p3.z } / v3{ 16 },
                    };

                    for (u32 i = 0; i < 3; i++)
                    {
                        const auto& vertex = triangle[i];
                        const v3i vertex_pos{ vertex.x, vertex.y, vertex.z };
                        const v3& vertex_pos_world = physics_triangle[i];

                        // Draw dot
                        renderer.draw_sphere(vertex_pos_world, 0.05f, v4{ 0.1f, 0.1f, 0.1f, 1.0f });

                        // Allow selecting dot
                        selection_mgr.add(phys::Sphere{ vertex_pos_world, 0.05f },
                            [this, vertex_pos, vertex_pos_world]
                                (const HoverData& hover_data)
                            {
                                if (hover_data.click_state.pressed_this_frame)
                                {
                                    m_selection = VertexPositionSelection{ vertex_pos };
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

        if (const auto selection = get_if<VertexPositionSelection>(&m_selection))
        {
            const v3i new_vertex_position{ m_gizmo.position() * 16.0f };
            for (auto& side: block_model.occluded_faces_per_side)
            {
                for (auto& face: side)
                {
                    for (auto& triangle: face)
                    {
                        for (auto& vertex: triangle)
                        {
                            const v3i vertex_pos{vertex.x, vertex.y, vertex.z};
                            if (vertex_pos == selection->vertex_pos)
                            {
                                vertex.x = new_vertex_position.x;
                                vertex.y = new_vertex_position.y;
                                vertex.z = new_vertex_position.z;
                                should_refresh_model = true;
                            }
                        }
                    }
                }
            }

            selection->vertex_pos = new_vertex_position;
        }

//        block_model.occluded_faces_per_side[m_selected_side_index][m_selected_face_index][m_selected_triangle_index]

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
            block_model.occluded_faces_per_side[side_index].push_back({});
        }
        else
        {
            block_model.unoccluded_faces.push_back({});
        }
    }

    void BlockModelEditor::create_triangle(u32 side_index, u32 face_index, h2o::BlockModel& block_model)
    {
        if (side_index < 6)
        {
            block_model.occluded_faces_per_side[side_index][face_index].push_back({});
            block_model.occluded_faces_per_side[side_index][face_index].push_back({});
            block_model.occluded_faces_per_side[side_index][face_index].push_back({});
        }
        else
        {
            block_model.unoccluded_faces[face_index].push_back({});
            block_model.unoccluded_faces[face_index].push_back({});
            block_model.unoccluded_faces[face_index].push_back({});
        }
    }
}