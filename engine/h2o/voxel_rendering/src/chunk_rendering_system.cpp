#include "voxel_rendering/chunk_rendering_system.h"

#include "core/engine.h"
#include "core/log.h"
#include "scene/scene.h"
#include "voxel/chunk.h"
#include "voxel/chunk_system.h"
#include "voxel_rendering/voxel_rendering_module.h"

namespace h2o
{
    ChunkRenderingSystem::ChunkRenderingSystem(const SceneSystemInitializer& system_initializer)
        : SceneSystem(system_initializer)
    {
        m_voxel_rendering_module = g_engine->get_module<VoxelRenderingModule>();
        assert(m_voxel_rendering_module);

        auto chunk_system = m_scene->get_system<ChunkSystem>();
        assert(chunk_system);

        // TODO: Create an event for when the chunk is fully generated; we probably don't want
        //       to create its mesh before then.
        chunk_system->on_chunk_created.add_listener(m_on_chunk_created_handle,
            [&](const ChunkEvent& event)
            {
                const v3i& chunk_pos = event.chunk.chunk_pos;
                assert(!m_mesh_index_map.contains(chunk_pos));

//                m_chunk_meshes.push_back(ChunkMesh(oof));
//                m_mesh_index_map[chunk_pos] = m_chunk_meshes.size() - 1;
            });

        chunk_system->on_chunk_deleted.add_listener(m_on_chunk_deleted_handle,
            [&](const ChunkEvent& event)
            {
//                event.chunk.chunk_pos
            });

        chunk_system->on_chunk_updated.add_listener(m_on_chunk_updated_handle,
            [&](const ChunkEvent& event)
            {

            });

        set_tick_phases(Render);
    }

    bool ChunkRenderingSystem::init()
    {
        return true;
    }

    void ChunkRenderingSystem::render(f32 delta_time)
    {
        for (const auto& mesh : m_chunk_meshes)
        {
//            mesh.
        }
    }
}