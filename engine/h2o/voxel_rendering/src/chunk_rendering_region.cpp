#include "voxel_rendering/chunk_rendering_region.h"

#include "core/log.h"
#include "voxel/chunk.h"
#include "voxel/chunk_system.h"
#include "voxel/direction.h"
#include "voxel/voxel_utils.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    ChunkRenderingRegion::ChunkRenderingRegion(
        gfx::IRenderer& renderer,
        const VoxelRenderingModule& voxel_rendering_module,
        const WeakHandle<ChunkSystem>& chunk_system)
        : ChunkRegion(chunk_system)
        , m_renderer(&renderer)
        , m_voxel_rendering_module(&voxel_rendering_module)
    {
        chunk_system->on_chunk_updated.add_listener(m_on_chunk_updated_handle,
            [&](const ChunkEvent& event)
            {
                const v3i& chunk_pos = event.chunk.chunk_pos();

                if (const auto local_pos = to_local_chunk_pos_3d(chunk_pos))
                {
                    const size_t index = to_index({ local_pos->x, local_pos->z });
                    if (const auto& mesh_column = m_chunk_mesh_columns[index])
                    {
                        auto& chunk_mesh = (*mesh_column)[chunk_pos.y];
                        chunk_mesh.update(voxel_rendering_module, event.chunk);
                    }
                }
            });

        set_tick_phases(TickPhase::Update);
    }

    void ChunkRenderingRegion::update(f32 delta_time)
    {
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

    std::array<Chunk*, 6> ChunkRenderingRegion::fetch_chunk_neighbours(const v3i& chunk_pos) const
    {
        std::array<Chunk*, 6> result{nullptr};
        for (size_t i = 0; i < 6; i++)
        {
            const auto direction = magic_enum::enum_value<voxel::Direction>(i);
            const v3i offset = voxel::to_vec3(direction);

            const v3i neighbour_pos = chunk_pos + offset;
            Chunk* neighbour = get_chunk_at(neighbour_pos);
            result[i] = neighbour;
        }

        return result;
    }

    bool ChunkRenderingRegion::are_surrounding_chunks_loaded(v2i chunk_pos) const
    {
        if (!get_chunk_col_at(chunk_pos + voxel::to_vec2(voxel::Direction::XNeg))) return false;
        if (!get_chunk_col_at(chunk_pos + voxel::to_vec2(voxel::Direction::XPos))) return false;
        if (!get_chunk_col_at(chunk_pos + voxel::to_vec2(voxel::Direction::ZNeg))) return false;
        if (!get_chunk_col_at(chunk_pos + voxel::to_vec2(voxel::Direction::ZPos))) return false;
        return true;
    }

    ChunkRenderingRegion::ChunkMeshColumn* ChunkRenderingRegion::get_chunk_mesh_col_at(v2i chunk_pos) const
    {
        if (const auto local_chunk_pos = to_local_chunk_pos_2d(chunk_pos))
        {
            const size_t index = to_index(*local_chunk_pos);
            return m_chunk_mesh_columns[index].get();
        }

        return nullptr;
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

    void ChunkRenderingRegion::on_chunk_fetched(const WeakHandle<ChunkColumn>& chunk_col, v2i local_chunk_pos)
    {
        const auto& chunk_mesh_col = m_chunk_mesh_columns[to_index(local_chunk_pos)];
        if (chunk_mesh_col)
            return;

        // Enqueue chunk for mesh update
        const v2i world_chunk_col_pos = local_chunk_pos + corner_pos();
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
                    glm::distance2(v2(world_chunk_col_pos), v2(center_pos())));
            }
        }
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

        i32 i = 0;
        for (auto& chunk_mesh : *new_column)
            chunk_mesh.init(*m_renderer, { chunk_col_pos.x, i++, chunk_col_pos.y });

        return new_column;
    }

    void ChunkRenderingRegion::update_next_chunk_mesh()
    {
        auto chunk_weak_handle = util::distance_queue_pop<ChunkWeakHandle>(m_chunks_to_mesh);

        if (!chunk_weak_handle)
            return;

        const auto chunk = chunk_weak_handle->chunk();
        if (!chunk)
            return;

        const v3i& chunk_pos = chunk->chunk_pos();
        const v2i chunk_col_pos { chunk_pos.x, chunk_pos.z };

        auto chunk_mesh_column = fetch_or_create_chunk_mesh_column(chunk_col_pos);
        if (!chunk_mesh_column)
            return;

        auto& chunk_mesh = (*chunk_mesh_column)[chunk_pos.y];

        chunk_mesh.update(*m_voxel_rendering_module, *chunk);
    }
}