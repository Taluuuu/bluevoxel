#include "voxel_rendering/chunk_rendering_system.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "scene/scene.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "voxel/chunk_mgr.h"

namespace h2o
{
    ChunkRenderingSystem::ChunkRenderingSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
        , m_voxel_rendering_module(g_engine->get_module<VoxelRenderingModule>())
    {
        auto rendering_module = g_engine->get_module<RenderingModule>();
        if (!rendering_module)
            return; // TODO: Give systems an init function

        auto& renderer = rendering_module->renderer();

        auto chunk_mgr = m_scene->get_system<ChunkMgr>();
        chunk_mgr->on_chunk_loaded.add_listener(m_on_chunk_loaded_handle,
            [](const ChunkEvent& event)
            {

            });

        chunk_mgr->on_chunk_updated.add_listener(m_on_chunk_updated_handle,
            [](const ChunkEvent& event)
            {

            });

        m_pipeline = renderer
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/chunk.vert")
            .add_shader(gfx::ShaderStage::Fragment, "Resources/engine/shaders/opengl/chunk.frag")
            .compile();

        set_tick_phases(Render);
    }

    void ChunkRenderingSystem::render(f32 delta_time)
    {

    }
}