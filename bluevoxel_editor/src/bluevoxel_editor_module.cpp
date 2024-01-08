#include "bluevoxel_editor_module.h"

#include "core/engine.h"
#include "game_framework/components/third_person_camera_component.h"
#include "game_framework/game_framework_module.h"
#include "input/input_component.h"
#include "input/input_module.h"
#include "rendering/camera.h"
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
    BlueVoxelEditorModule::BlueVoxelEditorModule()
        : h2o::Tickable(g_engine)
        , m_chunk_region({ 0, 0, 0 }, { 1, 1, 1 })
        , m_voxel_bounds({ 0, 0 }, 0)
    {}

    bool BlueVoxelEditorModule::init(h2o::Engine& engine)
    {
        m_rendering_module       = &engine.get_module_checked<h2o::RenderingModule>();
        m_ui_module              = &engine.get_module_checked<h2o::UIModule>();
        m_voxel_rendering_module = &engine.get_module_checked<h2o::VoxelRenderingModule>();
        auto& voxel_module       =  engine.get_module_checked<h2o::VoxelModule>();

        auto voxel_pack = engine
            .resource_mgr()
            .fetch<h2o::VoxelPack>("../Resources/bluevoxel/voxel/");

        if (!voxel_pack)
            return false;

        // Input setup
        m_input_module = &engine.get_module_checked<h2o::InputModule>();
        m_input_module->register_axis("move_x", h2o::Key::A, h2o::Key::D);
        m_input_module->register_axis("move_y", h2o::Key::S, h2o::Key::W);
        m_input_module->register_axis("fly", h2o::Key::LeftControl, h2o::Key::Space);
        m_input_module->register_axis("cam_x", h2o::MouseMoveDelta::Y, 0.2f, true);
        m_input_module->register_axis("cam_y", h2o::MouseMoveDelta::X, 0.2f, false);
        m_input_module->register_axis("cam_zoom", h2o::MouseScrollDelta::Y, 1.0f, false);

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

        voxel_module.set_voxel_pack(voxel_pack);

        m_chunk = std::make_shared<h2o::Chunk>(v3i{ 0, 0, 0 }, voxel_module);
        m_chunk->set_block_at({ 0, 0, 0 }, h2o::Block{ 1 });

        m_chunk_region.add_chunk(*m_chunk);

        m_chunk_mesh_pool = std::make_shared<h2o::ChunkMeshPool>();
        m_chunk_mesh_pool->build_chunk_mesh(m_chunk_region);
        m_chunk_mesh_pool->update_meshes(m_voxel_bounds);

        set_tick_phases(h2o::TickPhase::Update | h2o::TickPhase::Render);
        return true;
    }

    std::vector<std::type_index> BlueVoxelEditorModule::dependencies() const
    {
        return {
            typeid(h2o::UIModule),
            typeid(h2o::GameFrameworkModule),
            typeid(h2o::VoxelModule),
            typeid(h2o::VoxelRenderingModule) };
    }

    void BlueVoxelEditorModule::update(f32 delta_time)
    {
        m_ui_module->window("Editor", { { 50.0f, 50.0f }, { 250.0f, 250.0f } },
            [&](h2o::IUIRenderer& ui)
            {
                ui.row(25.0f, 1);

                if (ui.button("TEST"))
                    h2o::log::info("oui");
            }
        );
    }

    void BlueVoxelEditorModule::render()
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
                    renderer.draw_arrays(chunk_mesh.vertex_array, chunk_mesh.vertex_count);
                }
            }
        );
    }
}