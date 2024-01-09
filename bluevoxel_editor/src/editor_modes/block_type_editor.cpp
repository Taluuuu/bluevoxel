#include "block_type_editor.h"

#include "core/engine.h"
#include "game_framework/components/third_person_camera_component.h"
#include "input/input_component.h"
#include "rendering/pipeline.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/texture_array.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/ui_module.h"
#include "ui/ui_renderer.h"
#include "voxel/chunk.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_rendering/chunk_mesh_pool.h"
#include "voxel_rendering/voxel_rendering_module.h"

namespace bluevoxel
{
    BlockTypeEditor::BlockTypeEditor(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_chunk_region({ 0, 0, 0 }, { 1, 1, 1 })
        , m_voxel_bounds({ 0, 0 }, 0)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
        , m_ui_module(&g_engine->get_module_checked<h2o::UIModule>())
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
        , m_voxel_rendering_module(&g_engine->get_module_checked<h2o::VoxelRenderingModule>())
    {
        m_scene = std::make_shared<h2o::Scene>("editor_scene", nullptr);
        m_scene->add_system<h2o::RenderingSystem>();

        // Spawn camera
        auto camera = m_scene->spawn_actor();
        camera->add_component<h2o::InputComponent>();
        auto tps_cam = camera->add_component<h2o::ThirdPersonCameraComponent>();
        tps_cam->hold_click_to_rotate = true;
        camera->tag_actor(h2o::ActorTag::LocalPlayer);
        camera->transform.position = { 0.5f, 0.5f, 0.5f };
        camera->transform.rotation = { 0.0f, 0.0f, 90.0f };

        // Chunk setup
        m_chunk = std::make_shared<h2o::Chunk>(v3i{ 0, 0, 0 }, *m_voxel_module);
        m_chunk->set_block_at({ 0, 0, 0 }, h2o::Block{ 1 });
        m_chunk_region.add_chunk(*m_chunk);
        m_chunk_mesh_pool = std::make_shared<h2o::ChunkMeshPool>();
        m_chunk_mesh_pool->build_chunk_mesh(m_chunk_region);
        m_chunk_mesh_pool->update_meshes(m_voxel_bounds);

        on_voxel_pack_changed();
        m_voxel_module->on_voxel_pack_changed.add_listener(m_on_voxel_pack_changed_event_handle,
            [this](const h2o::VoxelPackChangedEvent& event)
            {
                on_voxel_pack_changed();
            }
        );

        // This should be updatable on-demand
        update_texture_paths();
        update_preset_names();

        set_tick_phases(h2o::TickPhase::Update | h2o::TickPhase::Render);
    }

    void BlockTypeEditor::update(f32 delta_time)
    {
        const auto& voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return;

        auto edited_block_type = voxel_pack->block_types()[m_selected_block_id];

        m_ui_module->window("Block Type Editor", { { 50.0f, 50.0f }, { 350.0f, 700.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 1);

                // Block type picker
                if (ui.combobox(m_block_type_names_c_str, m_selected_block_id))
                    on_changed_block_type_selection(m_selected_block_id);

                if (ui.button("Save"))
                    voxel_pack->save();

                if (edited_block_type)
                {
                    // In the future, we should be able to change a block type's id
                    // ui.input_int("Block ID", m_selected_block_id);

                    ui.row(25.0f, 2);

                    ui.label("Block preset: ");
                    if (ui.combobox(m_block_preset_names_c_str, edited_block_type->preset_id))
                        voxel_pack->edit_block_type(m_selected_block_id, *edited_block_type);

                    ui.label("Block model: ");
                    if (ui.combobox(m_block_model_names_c_str, edited_block_type->model_id))
                        voxel_pack->edit_block_type(m_selected_block_id, *edited_block_type);

                    ui.row(25.0f, 1);

                    ui.label("Textures: ");
                    const auto& model = voxel_pack->block_models()[edited_block_type->model_id];
                    for (u32 i = 0; i < model.calculate_face_count(); i++)
                    {
                        if (ui.combobox(m_texture_names_c_str, edited_block_type->texture_ids[i]))
                            voxel_pack->edit_block_type(m_selected_block_id, *edited_block_type);
                    }
                }
            }
        );
    }

    void BlockTypeEditor::render()
    {
        auto render_system = m_scene->get_system<h2o::RenderingSystem>();
        if (!render_system)
            return;

        const auto& camera = render_system->main_camera();
        if (!camera)
            return;

        const auto& pipeline = m_voxel_rendering_module->pipeline();
        const auto& block_textures = m_voxel_rendering_module->block_textures();

        auto& renderer = m_rendering_module->renderer();
        renderer.bind_pipeline(pipeline);

        // TODO: Avoid calculating proj_view matrix here.
        const m4 proj_view = camera->calc_proj_view();
        renderer.set_proj_view_matrix(proj_view); // TODO: This is a hack to allow drawing debug lines. Rework this ASAP.
        pipeline->set_uniform_mat4(0, proj_view);

        block_textures->bind(0);
        pipeline->set_uniform_int(2, 0);
        pipeline->set_uniform_vec3(3, glm::normalize(light_dir));
        pipeline->set_uniform_vec3(4, light_color);
        pipeline->set_uniform_float(5, ambient_strength);

        m_chunk_mesh_pool->for_each_chunk_mesh(
            [&](const h2o::ChunkMeshData& chunk_mesh)
            {
                if (chunk_mesh.vertex_count > 0)
                {
                    pipeline->set_uniform_ivec3(1, chunk_mesh.chunk_pos);
                    renderer.draw_arrays(chunk_mesh.vertex_array, chunk_mesh.vertex_count, h2o::gfx::DrawMode::Triangles);
                }
            }
        );

        // Draw gizmo
        renderer.draw_debug_line(v3(-0.5f), { 1.5f,-0.5f,-0.5f }, { 1.0f, 0.0f, 0.0f, 1.0f });
        renderer.draw_debug_line(v3(-0.5f), {-0.5f,-0.5f, 1.5f }, { 0.0f, 0.0f, 1.0f, 1.0f });
        renderer.draw_debug_line(v3(-0.5f), {-0.5f, 1.5f,-0.5f }, { 0.0f, 1.0f, 0.0f, 1.0f });
    }

    void BlockTypeEditor::on_changed_block_type_selection(h2o::BlockID block_id)
    {
        m_chunk->set_block_at({ 0, 0, 0 },
            h2o::Block{ block_id });

        m_chunk_mesh_pool->build_chunk_mesh(m_chunk_region);
        m_chunk_mesh_pool->update_meshes(m_voxel_bounds);
    }

    void BlockTypeEditor::on_voxel_pack_changed()
    {
        on_voxel_pack_updated();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            voxel_pack->on_voxel_pack_updated.add_listener(m_on_voxel_pack_updated_event_handle,
                [this](const h2o::VoxelPackUpdatedEvent& event)
                {
                    on_voxel_pack_updated();
                }
            );
        }
    }

    void BlockTypeEditor::on_voxel_pack_updated()
    {
        update_block_type_names();
        update_model_names();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            // Make sure the selected block id is valid
            m_selected_block_id = glm::min(size_t(m_selected_block_id), voxel_pack->block_types().size() - 1);
            on_changed_block_type_selection(m_selected_block_id);
        }
    }

    void BlockTypeEditor::update_block_type_names()
    {
        m_block_type_names_c_str.clear();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            const auto& block_types = voxel_pack->block_types();
            m_block_type_names_c_str.reserve(block_types.size());

            for (const auto& block_type : block_types)
            {
                if (block_type)
                {
                    m_block_type_names_c_str.push_back(block_type->name.c_str());
                }
                else
                {
                    m_block_type_names_c_str.push_back("none");
                }
            }
        }
    }

    void BlockTypeEditor::update_texture_paths()
    {
        m_texture_names_c_str.clear();
        m_texture_names.clear();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            const auto textures_path = voxel_pack->path() / h2o::VoxelPack::textures_folder_name;

            for (const auto& texture_path : fs::directory_iterator(textures_path))
                m_texture_names.push_back(texture_path.path().filename().string());

            m_texture_names_c_str.reserve(m_texture_names.size());
            for (const auto& texture_path_string : m_texture_names)
                m_texture_names_c_str.push_back(texture_path_string.c_str());
        }
    }

    void BlockTypeEditor::update_preset_names()
    {
        m_block_preset_names_c_str.clear();

        const auto& block_presets = m_voxel_module->block_presets();\

        m_block_preset_names_c_str.reserve(block_presets.size());
        for (const auto& block_preset : block_presets)
            m_block_preset_names_c_str.push_back(block_preset.name.c_str());
    }

    void BlockTypeEditor::update_model_names()
    {
        m_block_model_names_c_str.clear();

        if (const auto& voxel_pack = m_voxel_module->voxel_pack())
        {
            const auto& block_models = voxel_pack->block_models();

            m_block_model_names_c_str.reserve(block_models.size());
            for (const auto& block_model: block_models)
                m_block_model_names_c_str.push_back(block_model.name.c_str());
        }
    }
}