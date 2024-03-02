#include "uv_editor.h"

#include "core/engine.h"
#include "input/input_module.h"
#include "rendering/texture.h"
#include "ui/imgui.h"
#include "voxel/block_model.h"
#include "voxel/voxel_pack.h"
#include "voxel/block_type.h"

namespace bluevoxel
{
    UVEditor::UVEditor()
        : m_input_module(&g_engine->get_module_checked<h2o::InputModule>())
    {}

//    // Really slow and bad, need to cache the texture
//    static std::shared_ptr<h2o::gfx::Texture> get_selection_texture(
//        const h2o::VoxelPack& voxel_pack,
//        const h2o::BlockType& block_type,
//        const TriangleSelection& selection)
//    {
//        const u32 texture_id = block_type.texture_ids[selection.triangle_handle->face_handle.face_index];
//        const auto& texture_ids = voxel_pack.texture_ids();
//        const auto it = std::find_if(texture_ids.begin(), texture_ids.end(),
//            [&](const auto& item) -> bool
//            { return texture_id == item.second; });
//
//        if (it != texture_ids.end())
//        {
//            const auto texture_path = (voxel_pack.path() / h2o::VoxelPack::textures_folder_name / it->first).string();
//            return g_engine->resource_mgr().fetch<h2o::gfx::Texture>(texture_path);
//        }
//
//        return nullptr;
//    }

    bool UVEditor::update(
        h2o::UncookedBlockModel& block_model,
        const h2o::VoxelPack& voxel_pack,
        const h2o::BlockType& block_type)
    {
//        if (!ImGui::CollapsingHeader("UV Editor", ImGuiTreeNodeFlags_DefaultOpen))
//            return false;
//
//        const auto& [selected_triangle_handle, vertex_indices] = selection;
//
//        if (!selected_triangle_handle)
//            return false;
//
//        const auto tex = get_selection_texture(voxel_pack, block_type, selection);
//        if (!tex)
//            return false;
//
//        const v2 tex_pos = v2{ ImGui::GetCursorScreenPos() } + texture_offset;
//        const v2 tex_size { m_texture_size };
//        const v2 mouse_pos = ImGui::GetMousePos();
//
//        auto draw_list = ImGui::GetWindowDrawList();
//        draw_list->AddImage(
//            (void*)(u64)tex->id(),
//            tex_pos, tex_pos + tex_size);
//
//        const auto to_window_pos =
//            [&](v2i uv)
//            {
//                return tex_pos + tex_size * (v2{ uv } / f32(h2o::voxel_constants::max_coord_value_per_block));
//            };
//
//        const auto to_uv =
//            [&](v2 window_pos)
//            {
//                const v2 normalized_pos = (window_pos - tex_pos) / tex_size;
//                // +0.5 cuz we want the point to move around the center of the pixel and not its top-left
//                const v2i uv = normalized_pos * f32(h2o::voxel_constants::max_coord_value_per_block) + v2{ 0.5f };
//                return glm::clamp(uv, 0, h2o::voxel_constants::max_uv_value_per_block);
//            };
//
//        std::array<v2, 3> uv_positions{};
//        if (const auto triangle = block_model.get_triangle(*selected_triangle_handle))
//        {
//            for (u32 i = 0; i < 3; i++)
//            {
//                const auto& vertex = (*triangle)[i];
//                uv_positions[i] = to_window_pos(vertex.uv);
//            }
//        }
//
//        ImGui::InvisibleButton("UV Editor Canvas", tex_size + 2.0f * texture_offset, ImGuiButtonFlags_MouseButtonLeft);
//        if (ImGui::IsItemActive())
//        {
//            // Did we just start holding left click ?
//            if (m_input_module->mouse_button_state(h2o::MouseButton::Left).pressed_this_frame)
//            {
//                // Capture point if clicked
//                for (u32 i = 0; i < 3; i++)
//                {
//                    const v2 point_pos = uv_positions[i];
//                    if (glm::distance(mouse_pos, point_pos) < point_radius)
//                        m_selected_vertex_index = i;
//                }
//            }
//        }
//
//        // Move selected UV coordinate
//        bool uv_has_changed = false;
//        if (m_selected_vertex_index)
//        {
//            if (m_input_module->mouse_button_state(h2o::MouseButton::Left).held)
//            {
//                // Move selected point
//                const v2i new_uv = to_uv(mouse_pos);
//                if (auto triangle = block_model.get_triangle(*selection.triangle_handle))
//                {
//                    (*triangle)[*m_selected_vertex_index].uv = new_uv;
//                    uv_has_changed = true;
//                }
//            }
//            else
//            {
//                // Free point
//                m_selected_vertex_index = std::nullopt;
//            }
//        }
//
//        // Draw lines
//        for (u32 i = 0; i < 3; i++)
//        {
//            const v2 point_pos = uv_positions[i];
//            const v2 next_point_pos = uv_positions[(i + 1) % 3];
//
//            draw_list->AddLine(point_pos, next_point_pos, ImColor{ 1.0f, 1.0f, 0.0f, 1.0f }, 2.0f);
//        }
//
//        // Draw points
//        for (u32 i = 0; i < 3; i++)
//        {
//            const v2 point_pos = uv_positions[i];
//
//            const ImColor point_color = selection.vertex_indices.contains(i) ?
//                IM_COL32(255, 255, 0, 255) : IM_COL32(0, 0, 0, 255);
//
//            draw_list->AddCircleFilled(point_pos, point_radius, IM_COL32(255, 255, 255, 255));
//            draw_list->AddCircleFilled(point_pos, point_radius * 0.75f, point_color);
//        }
//
//        return uv_has_changed;
        return false;
    }
}