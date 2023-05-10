#include "voxel_rendering/chunk_rendering_system.h"

#include "core/engine.h"
#include "rendering/rendering_module.h"
#include "rendering/renderer.h"
#include "rendering/pipeline.h"
#include "voxel_rendering/voxel_rendering_module.h"

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

        m_pipeline = renderer
            .create_pipeline()
            .add_shader(gfx::ShaderStage::Vertex,   "Resources/engine/shaders/opengl/chunk.vert")
            .add_shader(gfx::ShaderStage::Fragment, "Resources/engine/shaders/opengl/chunk.frag")
            .compile();

        set_tick_phases(Render);
    }
}