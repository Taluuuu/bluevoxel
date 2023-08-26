#include "voxel_rendering/chunk_rendering_region.h"

#include "core/log.h"
#include "voxel/chunk_column.h"
#include "voxel/chunk_system.h"
#include "voxel/direction.h"
#include "voxel/voxel_utils.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    ChunkRenderingRegion::ChunkRenderingRegion(
        gfx::IRenderer& renderer,
        const VoxelRenderingModule& voxel_rendering_module,
        ChunkSystem& chunk_system)
        : ChunkRegion(chunk_system)
        , m_renderer(&renderer)
        , m_voxel_rendering_module(&voxel_rendering_module)
    {
        chunk_system.on_chunk_updated.add_listener(m_on_chunk_updated_handle,
            [&](const ChunkUpdateEvent& event)
            {
                Chunk* chunk = event.chunk_handle.chunk();

                assert(chunk);

                const v3i& chunk_pos = chunk->chunk_pos();

                util::distance_queue_insert(
                    m_chunks_to_mesh,
                    event.chunk_handle,
                    glm::distance2(v2(chunk_pos.x, chunk_pos.z), v2(center_pos())));
            }
        );

        set_tick_phases(TickPhase::Update);
    }

    void ChunkRenderingRegion::update(f32 delta_time)
    {
        remove_out_of_range_chunk_mesh_requests();

        for (i32 i = 0; i < 5; i++)
            update_next_chunk_mesh();
    }

    void ChunkRenderingRegion::for_each_chunk_mesh(const std::function<void(ChunkMesh&)>& fun) const
    {
        for (auto& chunk_mesh_column : m_chunk_mesh_columns)
        {
            if (chunk_mesh_column)
            {
                for (auto& chunk_mesh : *chunk_mesh_column)
                    fun(chunk_mesh);
            }
        }
    }

    void ChunkRenderingRegion::on_indices_changed(const std::vector<i32>& new_to_old_indices)
    {
        assert(m_voxel_rendering_module);

        std::vector<ChunkMeshColumnOwner> new_chunk_meshes(new_to_old_indices.size());

        for (i32 i = 0; i < size(); i++)
        for (i32 k = 0; k < size(); k++)
        {
            const i32 new_idx = i * i32(size()) + k;
            const i32 old_idx = new_to_old_indices[new_idx];

            if (old_idx != -1)
                new_chunk_meshes[new_idx] = std::move(m_chunk_mesh_columns[old_idx]);
        }

        m_chunk_mesh_columns = std::move(new_chunk_meshes);
    }

    void ChunkRenderingRegion::on_chunk_fetched(const WeakHandle<ChunkColumn>& chunk_col)
    {
        // Enqueue chunk for mesh update
        for (i32 j = 0; j < voxel_constants::vertical_chunk_count; j++)
        {
            const auto& chunk = (*chunk_col)[j];

            if (chunk.is_empty())
                continue; // Empty chunk; no need to mesh.

            ChunkWeakHandle chunk_weak_handle { chunk_col, j };
            if (!util::distance_queue_contains(m_chunks_to_mesh, chunk_weak_handle))
            {
                util::distance_queue_insert(
                    m_chunks_to_mesh,
                    chunk_weak_handle,
                    glm::distance2(v2(chunk_col->chunk_column_pos()), v2(center_pos())));
            }
        }

//        const v2i world_chunk_pos = chunk_col->chunk_column_pos();
//
//        // Enqueue chunk for mesh update
//        for (i32 j = 0; j < voxel_constants::vertical_chunk_count; j++)
//        {
//            const auto& chunk = (*chunk_col)[j];
//
//            if (chunk.is_empty())
//                continue; // Empty chunk; no need to mesh.
//
//            ChunkWeakHandle chunk_weak_handle { chunk_col, j };
//            if (!util::distance_queue_contains(m_chunks_to_mesh, chunk_weak_handle))
//            {
//                util::distance_queue_insert(
//                    m_chunks_to_mesh,
//                    chunk_weak_handle,
//                    glm::distance2(v2(world_chunk_pos), v2(center_pos())));
//            }
//        }

//        const auto& chunk_mesh_col = m_chunk_mesh_columns[to_index(local_chunk_pos)];
//        if (chunk_mesh_col)
//            return;
//
//        // Enqueue chunk for mesh update
//        const v2i world_chunk_col_pos = local_chunk_pos + corner_pos();
//        for (i32 j = 0; j < voxel_constants::vertical_chunk_count; j++)
//        {
//            const auto& chunk = (*chunk_col)[j];
//
//            if (chunk.is_empty())
//                continue; // Empty chunk; no need to mesh.
//
//            ChunkWeakHandle chunk_weak_handle { chunk_col, j };
//            if (!util::distance_queue_contains(m_chunks_to_mesh, chunk_weak_handle))
//            {
//                util::distance_queue_insert(
//                    m_chunks_to_mesh,
//                    chunk_weak_handle,
//                    glm::distance2(v2(world_chunk_col_pos), v2(center_pos())));
//            }
//        }
    }

    ChunkRenderingRegion::ChunkMeshColumn* ChunkRenderingRegion::fetch_or_create_chunk_mesh_column(v2i chunk_col_pos)
    {
        if (auto chunk_mesh_col = fetch_chunk_mesh_column(chunk_col_pos))
            return chunk_mesh_col;

        auto local_chunk_col_pos = to_local_chunk_pos_2d(chunk_col_pos);
        if (!local_chunk_col_pos)
            return nullptr;

        auto chunk_mesh_col = create_chunk_mesh_column(chunk_col_pos);
        assert(chunk_mesh_col);

        auto chunk_mesh_col_ptr = chunk_mesh_col.get();
        m_chunk_mesh_columns[to_index(*local_chunk_col_pos)] = std::move(chunk_mesh_col);

        return chunk_mesh_col_ptr;
    }

    ChunkRenderingRegion::ChunkMeshColumn* ChunkRenderingRegion::fetch_chunk_mesh_column(v2i chunk_col_pos) const
    {
        auto local_chunk_pos = to_local_chunk_pos_2d(chunk_col_pos);
        if (!local_chunk_pos)
            return nullptr;

        return m_chunk_mesh_columns[to_index(*local_chunk_pos)].get();
    }

    ChunkRenderingRegion::ChunkMeshColumnOwner ChunkRenderingRegion::create_chunk_mesh_column(v2i chunk_col_pos) const
    {
        auto new_column = std::make_unique<ChunkMeshColumn>();

        assert(m_voxel_rendering_module);

        i32 i = 0;
        for (auto& chunk_mesh : *new_column)
        {
            chunk_mesh.init(
                *m_voxel_rendering_module,
                *m_renderer,
                { chunk_col_pos.x, i++, chunk_col_pos.y });
        }

        return new_column;
    }

    bool ChunkRenderingRegion::fetch_adjacent_chunks(v3i chunk_pos, std::array<Chunk*, 6>& out_adj_chunks) const
    {
        // TODO: This can surely be replaced with a static chunk region
        for (u32 dir = 0; dir < 6; dir++)
        {
            const v3i offset = voxel::to_vec3(dir);

            const i32 y = offset.y + chunk_pos.y;
            if (y < 0 || y >= voxel_constants::vertical_chunk_count)
                continue;

            const auto adj_chunk_col = chunk_system().fetch_chunk_column(
                v2i{ chunk_pos.x, chunk_pos.z } + v2i{ offset.x, offset.z });

            if (!adj_chunk_col || !adj_chunk_col->is_generated())
                return false;

            out_adj_chunks[dir] = &(*adj_chunk_col)[y];
        }

        return true;
    }

    void ChunkRenderingRegion::remove_out_of_range_chunk_mesh_requests()
    {
        while (util::distance_queue_pop<ChunkWeakHandle>(m_chunks_to_mesh,
            [&](const ChunkWeakHandle& elem)
            {
                assert(elem.chunk());
                return !in_region_bounds(elem.chunk()->chunk_pos());
            })
        );
    }

    void ChunkRenderingRegion::update_next_chunk_mesh()
    {
        // Get the closest chunk that has all its neighbours loaded. Get its adjacent chunks.
        std::array<Chunk*, 6> adj_chunks { nullptr };
        auto chunk_weak_handle = util::distance_queue_pop<ChunkWeakHandle>(m_chunks_to_mesh,
            [&](const ChunkWeakHandle& elem) -> bool
            {
                assert(elem.chunk());

                adj_chunks.fill(nullptr);
                return fetch_adjacent_chunks(elem.chunk()->chunk_pos(), adj_chunks);
            }
        );

        if (!chunk_weak_handle)
            return;

        const auto chunk = chunk_weak_handle->chunk();
        assert(chunk);

        const v3i& chunk_pos = chunk->chunk_pos();
        const v2i chunk_col_pos { chunk_pos.x, chunk_pos.z };

        auto chunk_mesh_column = fetch_or_create_chunk_mesh_column(chunk_col_pos);
        if (!chunk_mesh_column)
            return;

        auto& chunk_mesh = (*chunk_mesh_column)[chunk_pos.y];

        chunk_mesh.update(*chunk, adj_chunks);
    }
}