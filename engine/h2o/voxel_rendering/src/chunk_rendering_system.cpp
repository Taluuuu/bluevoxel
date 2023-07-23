#include "voxel_rendering/chunk_rendering_system.h"

#include "core/engine.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "voxel/chunk.h"
#include "voxel/chunk_system.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "voxel_rendering/chunk_rendering_region.h"
#include "rendering/pipeline.h"
#include "rendering/camera.h"
#include "rendering/texture_array.h"
#include "scene_rendering/rendering_scene_system.h"
#include "imgui.h"

namespace h2o
{
    ChunkRenderingSystem::ChunkRenderingSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        m_voxel_rendering_module = g_engine->get_module<VoxelRenderingModule>();
        auto rendering_module = g_engine->get_module<RenderingModule>();
        assert(m_voxel_rendering_module && rendering_module);

        auto chunk_system = m_scene->get_system<ChunkSystem>();
        assert(chunk_system);

        m_renderer = &rendering_module->renderer();

        assert(m_renderer);
        m_chunk_rendering_region = std::make_unique<ChunkRenderingRegion>(*m_renderer, *m_voxel_rendering_module, chunk_system);
        m_chunk_rendering_region->set_size(5);

        chunk_system->on_player_changed_chunk.add_listener(m_on_player_changed_chunk_handle,
            [&](const PlayerChangedChunkEvent& event)
            {
                if (event.old_chunk_pos.x == event.new_chunk_pos.x &&
                    event.old_chunk_pos.z == event.new_chunk_pos.z)
                    return;

                m_chunk_rendering_region->set_corner_pos({
                    event.new_chunk_pos.x - m_chunk_rendering_region->size() / 2,
                    event.new_chunk_pos.z - m_chunk_rendering_region->size() / 2 });
            });

        set_tick_phases(Update | Render);
    }

    void ChunkRenderingSystem::update(f32 delta_time)
    {
        ImGui::Text("Voxel Settings");

        i32 new_size = i32(m_chunk_rendering_region->size());
        if (ImGui::SliderInt("World Size", &new_size, 0, 32))
            m_chunk_rendering_region->set_size(new_size);
    }

    void ChunkRenderingSystem::render(f32 delta_time)
    {
        auto render_system = m_scene->get_system<RenderingSystem>();
        if (!render_system)
            return;

        const auto& camera = render_system->main_camera();
        if (!camera)
            return;

        // TODO: Baddd
        const m4 proj_view = camera->calc_proj_view();

        // Don't null check these, always valid
        const auto& pipeline = m_voxel_rendering_module->pipeline();
        const auto& block_textures = m_voxel_rendering_module->block_textures();

        m_renderer->bind_pipeline(pipeline);
        pipeline->set_uniform_mat4(0, proj_view);

        block_textures->bind(0);
        pipeline->set_uniform_int(2, 0);

        assert(m_chunk_rendering_region);
        m_chunk_rendering_region->for_each_chunk_mesh(
            [&](const ChunkMesh& chunk_mesh)
            {
                pipeline->set_uniform_ivec3(1, chunk_mesh.chunk_pos());
                m_renderer->draw(chunk_mesh.vertex_array(), chunk_mesh.vertex_count());
            });
    }
}