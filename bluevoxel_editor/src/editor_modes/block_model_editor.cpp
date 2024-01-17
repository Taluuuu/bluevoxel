#include "block_model_editor.h"

#include "block_editor_workspace.h"
#include "core/engine.h"
#include "input/input_module.h"
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
        , m_workspace(&workspace)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {
        set_tick_phases(h2o::TickPhase::Update);
    }

    // https://antongerdelan.net/opengl/raycasting.html
    static v3 screen_to_ray(v2 screen_pos, v2 window_size, const m4& view, const m4& proj)
    {
        const v3 ray_nds{
            (2.0f * screen_pos.x) / window_size.x - 1.0f,
            1.0f - (2.0f * screen_pos.y) / window_size.y, 1.0f };

        const v4 ray_clip{ ray_nds.x, ray_nds.y, -1.0f, 1.0f };

        v4 ray_eye = glm::inverse(proj) * ray_clip;
        ray_eye = v4{ ray_eye.x, ray_eye.y, -1.0f, 0.0f };

        return glm::normalize(glm::inverse(view) * ray_eye);
    }

    std::optional<std::pair<f32, f32>> quadratic(f32 a, f32 b, f32 c)
    {
        const f32 discriminant = b * b - 4.0f * a * c;

        if (discriminant >= 0)
        {
            const f32 r1 = (-b + sqrt(discriminant)) / (2.0f * a);
            const f32 r2 = (-b - sqrt(discriminant)) / (2.0f * a);

            return std::pair<f32, f32>{ r1, r2 };
        }

        return std::nullopt;
    }

    static std::optional<f32> intersect_plane(
        const v3& ray_start, const v3& ray_dir,
        const v3& plane_point, const v3& plane_normal)
    {
        static constexpr f32 e = glm::epsilon<f32>();
        const f32 denom = glm::dot(ray_dir, plane_normal);
        if (denom < e)
            return std::nullopt;

        const f32 t = glm::dot(plane_point - ray_start, plane_normal) / denom;
        return (t >= 0.0f) ? std::optional<f32>{ t } : std::nullopt;
    }

    static std::optional<f32> intersect_disk(
        const v3& ray_start, const v3& ray_dir,
        const v3& disk_center, const v3& disk_normal, f32 disk_radius)
    {
        if (const auto t = intersect_plane(ray_start, ray_dir, disk_center, disk_normal))
        {
            const v3 plane_intersection = ray_start + ray_dir * *t;
            const v3 plane_intersection_to_disk_center = disk_center - plane_intersection;
            const f32 distance_sqr = glm::dot(plane_intersection_to_disk_center, plane_intersection_to_disk_center);
            const f32 radius_sqr = disk_radius * disk_radius;
            return (distance_sqr <= radius_sqr) ? t : std::nullopt;
        }

        return std::nullopt;
    }

    // https://hugi.scene.org/online/hugi24/coding%20graphics%20chris%20dragan%20raytracing%20shapes.htm
    // https://mrl.cs.nyu.edu/~dzorin/rendering/lectures/lecture3/lecture3.pdf
    static std::optional<f32> intersect_cylinder(
        const v3& ray_start, const v3& ray_dir,
        const v3& p1, const v3& p2, f32 radius)
    {
        const v3 C = p2;
        const v3 D = ray_dir;
        const v3 V = glm::normalize(p1 - p2);
        const v3 O = ray_start;
        const v3 X = O - C;

        const f32 d_dot_v = glm::dot(D, V);
        const f32 x_dot_v = glm::dot(X, V);

        const f32 a = glm::dot(D, D) - d_dot_v * d_dot_v;
        const f32 b = 2.0f * (glm::dot(D, X) - glm::dot(D, V) * glm::dot(X, V));
        const f32 c = glm::dot(X, X) - x_dot_v * x_dot_v - radius * radius;

        std::array<f32, 4> t_values{};
        size_t size = 0;

        if (const auto temp = quadratic(a, b, c))
        {
            const f32 t1 = temp->first;
            const f32 t2 = temp->second;

            const f32 maxm = glm::distance(p1, p2);
            const f32 m1 = d_dot_v * t1 + x_dot_v;
            const f32 m2 = d_dot_v * t2 + x_dot_v;

            if (t1 >= 0.0f && m1 >= 0.0f && m1 <= maxm)
                t_values[size++] = t1;

            if (t2 >= 0.0f && m2 >= 0.0f && m2 <= maxm)
                t_values[size++] = t2;
        }

        if (const auto t = intersect_disk(ray_start, ray_dir, p1, -V, radius))
            t_values[size++] = *t;

        if (const auto t = intersect_disk(ray_start, ray_dir, p2,  V, radius))
            t_values[size++] = *t;

        if (size == 0)
            return std::nullopt;

        f32 min_t = FLT_MAX;
        for (size_t i = 0; i < size; i++)
        {
            const f32 t = t_values[i];
            if (t < min_t)
                min_t = t;
        }

        return min_t;
    }

    void BlockModelEditor::update(f32 delta_time)
    {
        // WIP gizmo
        auto& renderer = m_rendering_module->renderer();
        renderer.draw_cylinder({0.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}, 0.1f, {1.0f, 0.0f, 0.0f, 1.0f});
        renderer.draw_cylinder({0.0f, 0.0f, 0.0f}, {0.0f, 2.0f, 0.0f}, 0.5f, {0.0f, 1.0f, 0.0f, 1.0f});
        renderer.draw_cylinder({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 2.0f}, 0.1f, {0.0f, 0.0f, 1.0f, 1.0f});

        const auto& window = g_engine->get_module_checked<h2o::WindowingModule>().window();
        const auto& input = g_engine->get_module_checked<h2o::InputModule>();
        const v3 ray_dir = screen_to_ray(
            input.mouse_position(),
            window.window_size(),
            renderer.view_matrix(),
            renderer.proj_matrix());

        if (const auto t = intersect_cylinder(renderer.camera().position(), ray_dir, {0.0f, 0.0f, 0.0f}, {0.0f, 2.0f, 0.0f}, 0.5f))
            h2o::log::info("{}", *t);

        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return;

        // TODO: Make easy function to get the block type in the workspace
        const auto& block_type = voxel_pack->block_types()[m_workspace->selected_block_id()];
        if (!block_type)
            return;

        const auto model_id = block_type->model_id;
        h2o::BlockModel block_model = voxel_pack->block_models()[model_id];

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
                edit_side("Occluded by X-", 0, block_model.occluded_faces_per_side[0]);
                edit_side("Occluded by X+", 1, block_model.occluded_faces_per_side[1]);
                edit_side("Occluded by Z-", 2, block_model.occluded_faces_per_side[2]);
                edit_side("Occluded by Z+", 3, block_model.occluded_faces_per_side[3]);
                edit_side("Occluded by Y-", 4, block_model.occluded_faces_per_side[4]);
                edit_side("Occluded by Y+", 5, block_model.occluded_faces_per_side[5]);
                edit_side("Unoccluded Faces", 6, block_model.unoccluded_faces);
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