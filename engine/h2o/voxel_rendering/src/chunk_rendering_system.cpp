#include "voxel_rendering/chunk_rendering_system.h"

#include "core/engine.h"
#include "core/log.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "voxel/chunk.h"
#include "voxel/chunk_system.h"
#include "voxel_rendering/chunk_mesh.h"
#include "voxel_rendering/voxel_rendering_module.h"
#include "rendering/pipeline.h"

namespace h2o
{
    ChunkRenderingSystem::ChunkRenderingSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        auto voxel_rendering_module = g_engine->get_module<VoxelRenderingModule>();
        auto rendering_module = g_engine->get_module<RenderingModule>();
        assert(voxel_rendering_module && rendering_module);

        auto chunk_system = m_scene->get_system<ChunkSystem>();
        assert(chunk_system);

        m_renderer = &rendering_module->renderer();

        // TODO: Create an event for when the chunk is fully generated; we probably don't want
        //       to create its mesh before then.
        chunk_system->on_chunk_created.add_listener(m_on_chunk_created_handle,
            [&](const ChunkEvent& event)
            {
                assert(m_renderer);
                create_mesh_at(*m_renderer, event.chunk.chunk_pos);
            });

        chunk_system->on_chunk_deleted.add_listener(m_on_chunk_deleted_handle,
            [&](const ChunkEvent& event)
            {
                erase_mesh_at(event.chunk.chunk_pos);
            });

        chunk_system->on_chunk_updated.add_listener(m_on_chunk_updated_handle,
            [&](const ChunkEvent& event)
            {
                const v3i& chunk_pos = event.chunk.chunk_pos;
                assert(m_mesh_index_map.contains(chunk_pos));

                // A bit sketchy, no checks
                m_chunk_meshes[m_mesh_index_map[chunk_pos]].update(event.chunk);
            });

        set_tick_phases(Render);
    }

    bool ChunkRenderingSystem::init()
    {
        assert(m_renderer && m_voxel_rendering_module);
        auto& pipeline = m_voxel_rendering_module->pipeline();
    }

    void ChunkRenderingSystem::render(f32 delta_time)
    {

        for (const auto& mesh : m_chunk_meshes)
        {
//            mesh.vertex_array();
        }
    }

    void ChunkRenderingSystem::create_mesh_at(gfx::IRenderer& renderer, const v3i& chunk_pos)
    {
        assert(!m_mesh_index_map.contains(chunk_pos));

        m_chunk_meshes.emplace_back(renderer, chunk_pos);
        m_mesh_index_map[chunk_pos] = m_chunk_meshes.size() - 1;
    }

    void ChunkRenderingSystem::erase_mesh_at(const v3i& chunk_pos)
    {
        assert(m_mesh_index_map.contains(chunk_pos));

        size_t index = m_mesh_index_map[chunk_pos];

        m_chunk_meshes[index] = std::move(m_chunk_meshes.back());
        m_chunk_meshes.pop_back();

        const auto& moved_mesh = m_chunk_meshes[index];
        m_mesh_index_map[moved_mesh.chunk_pos()] = index;

        m_mesh_index_map.erase(chunk_pos);
    }
}