#include "triangle_selection.h"

#include "block_editor/block_editor_workspace.h"
#include "block_model_editor.h"
#include "block_model_editor_constants.h"
#include "core/engine.h"
#include "input/input_module.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "ui/imgui.h"
#include "voxel/uncooked_block_model.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

namespace bluevoxel
{
    using FaceHandle = h2o::UncookedBlockModel::FaceHandle;
    using Face = h2o::UncookedBlockModel::Face;
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

        m_gizmo.increment_size = 1.0f / h2o::voxel_constants::max_coord_value_per_block;
        m_gizmo.bounds = Gizmo::Bounds{ v3{ 0.0f }, v3{ 1.0f } };
    }

    bool TriangleSelection::update(
        h2o::UncookedBlockModel& block_model,
        const h2o::BlockType& block_type,
        const h2o::VoxelPack& voxel_pack)
    {
        m_gizmo.set_enabled(!m_selection.vertex_indices.empty());
        const v3i gizmo_delta{ m_gizmo.movement_delta() * f32(h2o::voxel_constants::max_coord_value_per_block) };

        auto& renderer = m_rendering_module->renderer();
        const auto& camera = renderer.camera();
        const v3 camera_pos = camera.position();

        auto& selection_mgr = m_model_editor->workspace().selection_mgr();
        make_triangles_selectable(selection_mgr, block_model);

        bool should_rebuild_model = draw_triangle_ui(block_model);

        if (m_selection.triangle_handle)
        {
            for (u32 i = 0; i < 3; i++)
            {
                const bool is_selected = m_selection.vertex_indices.contains(i);

                const VertexHandle vertex_handle{*m_selection.triangle_handle, i};
                if (auto vertex = block_model.get_vertex(vertex_handle))
                {
                    const v3 vertex_world_pos = vertex->world_pos();

                    const f32 distance_with_camera = glm::length(camera_pos - vertex_world_pos);
                    const f32 vertex_radius_ = model_editor_constants::vertex_radius * distance_with_camera;

                    // Draw vertex
                    renderer.draw_sphere(
                        vertex_world_pos,
                        vertex_radius_,
                        is_selected ? model_editor_constants::selected_vertex_color
                            : model_editor_constants::unselected_vertex_color);

                    // Allow selecting vertex
                    selection_mgr.add(phys::Sphere{vertex_world_pos, vertex_radius_},
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
                                m_selection.vertex_indices = {i};
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

            if (m_uv_editor.update(
                block_model,
                voxel_pack,
                block_type,
                *m_selection.triangle_handle,
                m_selection.vertex_indices))
            {
                return true;
            }
        }

        return should_rebuild_model || gizmo_delta != v3i{};
    }

    void TriangleSelection::make_triangles_selectable(
        SelectionManager& selection_mgr,
        const h2o::UncookedBlockModel& block_model)
    {
        block_model.for_each_triangle(
            [&](const TriangleHandle& triangle_handle, const Triangle& triangle)
            {
                if (triangle.is_hidden)
                    return;

                const phys::Triangle physics_triangle{
                    triangle.vertices[0].world_pos(),
                    triangle.vertices[1].world_pos(),
                    triangle.vertices[2].world_pos(),
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
    }

    static void set_all_triangles_hidden(h2o::UncookedBlockModel& block_model, bool hidden)
    {
        block_model.for_each_triangle(
            [&](const TriangleHandle& triangle_handle, Triangle& triangle)
            {
                triangle.is_hidden = hidden;
            }
        );
    }

    bool TriangleSelection::draw_triangle_ui(h2o::UncookedBlockModel& block_model)
    {
        if (!ImGui::CollapsingHeader("Model Faces", ImGuiTreeNodeFlags_DefaultOpen))
            return false;

        bool should_rebuild_model = false;
        if (ImGui::Button("Hide all Triangles"))
        {
            set_all_triangles_hidden(block_model, true);
            should_rebuild_model = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Show all Triangles"))
        {
            set_all_triangles_hidden(block_model, false);
            should_rebuild_model = true;
        }

        if (ImGui::Button("Create Face"))
        {
            if (const auto& voxel_pack = g_engine->get_module_checked<h2o::VoxelModule>().voxel_pack())
                voxel_pack->add_face_to_model(block_model.id, {});
        }

        block_model.for_each_face(
            [&](const FaceHandle& face_handle, Face& face)
            {
                ImGui::BeginChild(
                    fmt::format("Face {}", face_handle.face_index).c_str(),
                    { 0, 100 }, true);

                ImGui::Text("Face %i", face_handle.face_index);

                ImGui::SameLine();

                if (ImGui::Button(fmt::format("Create Triangle###create_{}", face_handle.face_index).c_str()))
                {
                    block_model.add_triangle(face_handle, Triangle{
                        {
                            Vertex{ { 16, 0,  8 }, { 16, 16 } },
                            Vertex{ { 16, 16, 8 }, { 16, 0  } },
                            Vertex{ { 0,  16, 8 }, { 0,  0  } },
                        }, false });

                    should_rebuild_model = true;
                }

                ImGui::SameLine();

                if (ImGui::Button(fmt::format("Delete Face###delete_face_{}", face_handle.face_index).c_str()))
                {
                    if (const auto& voxel_pack = g_engine->get_module_checked<h2o::VoxelModule>().voxel_pack())
                        voxel_pack->remove_face_from_model(block_model.id, face_handle);

                    should_rebuild_model = true;
                }

                if (ImGui::BeginTable(fmt::format("Face {} Triangles", face_handle.face_index).c_str(), 5, ImGuiTableFlags_Borders))
                {
                    ImGui::TableNextRow();

                    for (u32 triangle_index = 0; triangle_index < face.size(); triangle_index++)
                    {
                        auto& triangle = face[triangle_index];
                        const TriangleHandle triangle_handle{ face_handle, triangle_index };

                        ImGui::TableNextColumn();
                        ImGui::Text("Triangle %i", triangle_index);

                        ImGui::TableNextColumn();
                        ImGui::BeginDisabled(triangle_handle == m_selection.triangle_handle);
                        if (ImGui::Button(fmt::format("Pick###pick_{}", triangle_index).c_str()))
                        {
                            m_selection.triangle_handle = triangle_handle;
                            m_selection.vertex_indices = {};
                        }
                        ImGui::EndDisabled();

                        ImGui::TableNextColumn();
                        if (ImGui::Checkbox(fmt::format("Hidden###hidden_{}", triangle_index).c_str(), &triangle.is_hidden))
                            should_rebuild_model = true;

                        ImGui::TableNextColumn();
                        if (ImGui::Button(fmt::format("Delete###delete_triangle_{}", triangle_index).c_str()))
                        {
                            block_model.delete_triangle(triangle_handle);
                            should_rebuild_model = true;
                        }

                        ImGui::TableNextColumn();
                        if (ImGui::Button(fmt::format("Up###move_up{}", triangle_index).c_str()))
                        {
                            const Triangle triangle_cpy = triangle;
                            if (triangle_index == 0)
                            {
                                if (face_handle.face_index > 0)
                                {
                                    block_model.add_triangle({ face_handle.face_index - 1 }, triangle_cpy);
                                    block_model.delete_triangle(triangle_handle);
                                    should_rebuild_model = true;
                                }
                            }
                            else
                            {
                                block_model.delete_triangle(triangle_handle);
                                face.insert(face.cbegin() + triangle_index - 1, triangle_cpy);
                                should_rebuild_model = true;
                            }
                        }

                        ImGui::SameLine();

                        if (ImGui::Button(fmt::format("Down###move_down{}", triangle_index).c_str()))
                        {
                            const Triangle triangle_cpy = triangle;
                            if (triangle_index == face.size() - 1)
                            {
                                if (face_handle.face_index < block_model.face_count() - 1)
                                {
                                    if (auto face_ = block_model.get_face({ face_handle.face_index + 1 }))
                                    {
                                        block_model.delete_triangle(triangle_handle);
                                        face_->insert(face_->cbegin(), triangle_cpy);
                                        should_rebuild_model = true;
                                    }
                                }
                            }
                            else
                            {
                                block_model.delete_triangle(triangle_handle);
                                face.insert(face.cbegin() + triangle_index + 1, triangle_cpy);
                                should_rebuild_model = true;
                            }
                        }
                    }

                    ImGui::EndTable();
                }

                ImGui::EndChild();
            }
        );

        return should_rebuild_model;
    }
}