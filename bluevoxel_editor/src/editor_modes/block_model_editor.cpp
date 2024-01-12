#include "block_model_editor.h"

#include "block_editor_workspace.h"
#include "core/engine.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

namespace bluevoxel
{
    BlockModelEditor::BlockModelEditor(BlockEditorWorkspace& workspace)
        : h2o::Tickable(&workspace)
        , m_workspace(&workspace)
        , m_ui_module(&g_engine->get_module_checked<h2o::UIModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
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

        const auto model_id = block_type->model_id;
        h2o::BlockModel block_model = voxel_pack->block_models()[model_id];

        bool should_refresh_model = false;

        m_ui_module->window("Block Model Editor", { { 1175.0f, 150.0f }, { 400.0f, 725.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 1);

                ui.label(fmt::format("Editing Model '{}'", block_model.name));

                ui.label("Faces:");

                ui.tree_push("Occluded By X-", [&]
                {
                    if (ui.button("Create Triangle"))
                    {
                        create_triangle();
                        should_refresh_model = true;
                    }

                    for (auto& face : block_model.occluded_faces_per_side[0])
                    {
                        for (size_t i = 0; i < face.size(); i += 3)
                        {
                            should_refresh_model = should_refresh_model ||
                                edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
                        }
                    }
                });

                ui.tree_push("Occluded By X+", [&]
                {
                    if (ui.button("Create Triangle"))
                    {
                        create_triangle();
                        should_refresh_model = true;
                    }

                    for (auto& face : block_model.occluded_faces_per_side[1])
                    {
                        for (size_t i = 0; i < face.size(); i += 3)
                        {
                            should_refresh_model = should_refresh_model ||
                                edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
                        }
                    }
                });

                ui.tree_push("Occluded By Y-", [&]
                {
                    if (ui.button("Create Triangle"))
                    {
                        create_triangle();
                        should_refresh_model = true;
                    }

                    for (auto& face : block_model.occluded_faces_per_side[2])
                    {
                        for (size_t i = 0; i < face.size(); i += 3)
                        {
                            should_refresh_model = should_refresh_model ||
                                edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
                        }
                    }
                });

                ui.tree_push("Occluded By Y+", [&]
                {
                    if (ui.button("Create Triangle"))
                    {
                        create_triangle();
                        should_refresh_model = true;
                    }

                    for (auto& face : block_model.occluded_faces_per_side[3])
                    {
                        for (size_t i = 0; i < face.size(); i += 3)
                        {
                            should_refresh_model = should_refresh_model ||
                                edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
                        }
                    }
                });

                ui.tree_push("Occluded By Z-", [&]
                {
                    if (ui.button("Create Triangle"))
                    {
                        create_triangle();
                        should_refresh_model = true;
                    }

                    for (auto& face : block_model.occluded_faces_per_side[4])
                    {
                        for (size_t i = 0; i < face.size(); i += 3)
                        {
                            should_refresh_model = should_refresh_model ||
                                edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
                        }
                    }
                });

                ui.tree_push("Occluded By Z+", [&]
                {
                    if (ui.button("Create Triangle"))
                    {
                        create_triangle();
                        should_refresh_model = true;
                    }

                    for (auto& face : block_model.occluded_faces_per_side[5])
                    {
                        for (size_t i = 0; i < face.size(); i += 3)
                        {
                            should_refresh_model = should_refresh_model ||
                                edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
                        }
                    }
                });

//                    ui.tree_push("Unoccluded", [&]
//                    {
//                        if (ui.button("Create Triangle"))
//                            create_triangle();
//
//
//                        auto& face = block_model.unoccluded_faces[0];
//                        for (size_t i = 0; i < face.size(); i += 3)
//                            edit_triangle(ui, i / 3, face[i + 0], face[i + 1], face[i + 2]);
//                    });
            }
        );

        if (should_refresh_model)
        {
            voxel_pack->edit_block_model(model_id, block_model);
        }
    }

    bool BlockModelEditor::edit_triangle(h2o::IUIRenderer& ui, u32 triangle_index, h2o::BlockVertex& p0, h2o::BlockVertex& p1, h2o::BlockVertex& p2)
    {
        bool was_triangle_edited = false;

        ui.tree_push(fmt::format("Triangle {}", triangle_index), [&]
        {
            ui.row(25.0f, 1);
            ui.label("P0");

            was_triangle_edited = was_triangle_edited || edit_vertex(ui, p0);

            ui.row(25.0f, 1);
            ui.label("P1");

            was_triangle_edited = was_triangle_edited || edit_vertex(ui, p1);

            ui.row(25.0f, 1);
            ui.label("P2");

            was_triangle_edited = was_triangle_edited || edit_vertex(ui, p2);
        });

        return was_triangle_edited;
    }

    bool BlockModelEditor::edit_vertex(h2o::IUIRenderer& ui, h2o::BlockVertex& vertex)
    {
        u32 x = vertex.x;
        u32 y = vertex.y;
        u32 z = vertex.z;
        u32 u = vertex.u;
        u32 v = vertex.v;

        ui.row(25.0f, 3);

        bool was_vertex_edited =
            ui.input_uint("X: ", x) ||
            ui.input_uint("Y: ", y) ||
            ui.input_uint("Z: ", z);

        ui.row(25.0f, 2);

        was_vertex_edited = was_vertex_edited ||
            ui.input_uint("U: ", u) ||
            ui.input_uint("V: ", v);

        return was_vertex_edited;
    }

    void BlockModelEditor::create_triangle()
    {

    }
}