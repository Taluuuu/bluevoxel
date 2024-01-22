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
        , m_gizmo(this)
        , m_workspace(&workspace)
        , m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
        , m_window_module(&g_engine->get_module_checked<h2o::WindowingModule>())
    {
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

        const auto& window = m_window_module->window();
        auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();

        const auto model_id = block_type->model_id;
        h2o::BlockModel block_model = voxel_pack->block_models()[model_id];

        if (m_input_module->mouse_button_state(h2o::MouseButton::Left).pressed_this_frame)
        {
            const v3 mouse_ray_dir = h2o::physics::screen_to_ray_direction(
                m_input_module->mouse_position(),
                window.window_size(),
                renderer.view_matrix(),
                renderer.proj_matrix());

            const h2o::physics::Ray ray{camera.position(), mouse_ray_dir};

            u32 side_index = 0;
            f32 min_t = FLT_MAX;
            for (const auto& side: block_model.occluded_faces_per_side)
            {
                u32 face_index = 0;
                for (const auto& face: side)
                {
                    for (const auto& triangle: face)
                    {
                        const auto& p1 = triangle[0];
                        const auto& p2 = triangle[1];
                        const auto& p3 = triangle[2];

                        h2o::physics::Triangle physics_triangle{
                            .p1 = v3{p1.x, p1.y, p1.z} / v3{16},
                            .p2 = v3{p2.x, p2.y, p2.z} / v3{16},
                            .p3 = v3{p3.x, p3.y, p3.z} / v3{16},
                        };

                        if (const auto t = h2o::physics::intersect_triangle(ray, physics_triangle))
                        {
                            if (*t < min_t)
                            {
                                min_t = *t;
                                m_selected_side_index = side_index;
                                m_selected_face_index = face_index;

                                m_gizmo.set_position((physics_triangle.p1 + physics_triangle.p2 + physics_triangle.p3) / 3.0f);
                            }
                        }
                    }

                    face_index++;
                }

                side_index++;
            }
        }

        bool should_refresh_model = false;
        u32 vertex_index = 0;
        u32 triangle_index = 0;

        auto edit_side = [&](const std::string& side_name, u32 side_index, std::vector<std::vector<h2o::BlockVertex>>& faces)
        {
            if (ImGui::CollapsingHeader(side_name.c_str()))
            {
                if (ImGui::Button("Create Face"))
                {
                    create_face(side_index, block_model);
                    should_refresh_model = true;
                }

                for (i32 face_index = 0; face_index < faces.size(); face_index++)
                {
                    if (ImGui::CollapsingHeader(fmt::format("Face {}##{}", face_index, side_index).c_str()))
                    {
                        if (ImGui::Button("Create Triangle"))
                        {
                            create_triangle(side_index, face_index, block_model);
                            should_refresh_model = true;
                        }

                        if (ImGui::BeginTable(fmt::format("Triangle##{}", triangle_index++).c_str(), 5))
                        {
                            ImGui::TableHeadersRow();

                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("X");

                            ImGui::TableNextColumn();
                            ImGui::Text("Y");

                            ImGui::TableNextColumn();
                            ImGui::Text("Z");

                            ImGui::TableNextColumn();
                            ImGui::Text("U");

                            ImGui::TableNextColumn();
                            ImGui::Text("V");

                            auto& face = faces[face_index];
                            const u32 num_triangles = face.size() / 3;
                            for (u32 i = 0; i < num_triangles; i++)
                            {
                                ImGui::TableNextRow();
                                should_refresh_model = should_refresh_model || edit_vertex(vertex_index++, face[3 * i + 0]);

                                ImGui::TableNextRow();
                                should_refresh_model = should_refresh_model || edit_vertex(vertex_index++, face[3 * i + 1]);

                                ImGui::TableNextRow();
                                should_refresh_model = should_refresh_model || edit_vertex(vertex_index++, face[3 * i + 2]);
                            }

                            ImGui::EndTable();
                        }
                    }
                }
            }
        };

        if (ImGui::Begin("Block Model Editor"))
        {
            ImGui::Text("Editing Model '%s'", block_model.name.c_str());

            if (ImGui::CollapsingHeader("Faces"))
            {
//                edit_side("Occluded by X-", 0, block_model.occluded_faces_per_side[0]);
//                edit_side("Occluded by X+", 1, block_model.occluded_faces_per_side[1]);
//                edit_side("Occluded by Z-", 2, block_model.occluded_faces_per_side[2]);
//                edit_side("Occluded by Z+", 3, block_model.occluded_faces_per_side[3]);
//                edit_side("Occluded by Y-", 4, block_model.occluded_faces_per_side[4]);
//                edit_side("Occluded by Y+", 5, block_model.occluded_faces_per_side[5]);
//                edit_side("Unoccluded Faces", 6, block_model.unoccluded_faces);
            }
        }
        ImGui::End();

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