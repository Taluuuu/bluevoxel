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
        auto chunk_distance = voxel::chunk_distance_queue_pop(m_chunks_to_mesh,
            [&](const ChunkDistance& chunk) -> bool { return true; });

        if (!chunk_distance)
            return;

        auto chunk = chunk_distance->chunk;
        assert(chunk);

        const v3i& chunk_pos = chunk->chunk_pos();
        const v2i chunk_col_pos { chunk_pos.x, chunk_pos.z };
        const auto local_chunk_col_pos = to_local_chunk_pos_2d(chunk_col_pos);
        if (!local_chunk_col_pos)
            return;

        // Create chunk mesh column if it does not already exist.
        auto chunk_col_mesh = get_chunk_mesh_col_at(chunk_col_pos);
        if (!chunk_col_mesh)
        {
            auto new_column = std::make_unique<ChunkMeshColumn>();
            chunk_col_mesh = new_column.get();

            const size_t index = to_index(*local_chunk_col_pos);

            m_chunk_mesh_columns[index] = std::move(new_column);

            i32 i = 0;
            for (auto& chunk_mesh : *chunk_col_mesh)
                chunk_mesh.init(*m_renderer, { chunk_col_pos.x, i++, chunk_col_pos.y });
        }

        assert(chunk_col_mesh);

        auto& chunk_mesh = (*chunk_col_mesh)[chunk_pos.y];

        chunk_mesh.update(*m_voxel_rendering_module, *chunk);
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

        std::vector<ChunkMeshColumnPtr> new_chunk_meshes(new_to_old_indices.size());

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

    void ChunkRenderingRegion::on_chunk_fetched(const ChunkColumnPtr& chunk_col, v2i local_chunk_pos)
    {
        const auto& chunk_mesh_col = m_chunk_mesh_columns[to_index(local_chunk_pos)];
        if (chunk_mesh_col)
            return;

        const v2i world_chunk_col_pos = local_chunk_pos + corner_pos();
        for (i32 j = 0; j < voxel_constants::vertical_chunk_count; j++)
        {
            if ((*chunk_col)[j].is_empty())
                continue;

            const v3 chunk_world_pos = chunk_to_world_pos({world_chunk_col_pos.x, j, world_chunk_col_pos.y});

            const ChunkDistance chunk_distance{
                &(*chunk_col)[j],
                glm::distance2(chunk_world_pos, player_pos)};

            // Enqueue chunk meshing
            voxel::chunk_distance_queue_insert(m_chunks_to_mesh, chunk_distance);
        }

//        for (i32 i = 0; i < size(); i++)
//        for (i32 k = 0; k < size(); k++)
//        {
//            const auto& chunk_mesh_col = m_chunk_mesh_columns[to_index({i, k})];
//            if (chunk_mesh_col)
//                continue;
//
//            const v2i world_chunk_col_pos = local_chunk_pos + corner_pos();
//            for (i32 j = 0; j < voxel_constants::vertical_chunk_count; j++)
//            {
//                if ((*chunk_col)[j].is_empty())
//                    continue;
//
//                const v3 chunk_world_pos = chunk_to_world_pos({ world_chunk_col_pos.x, j, world_chunk_col_pos.y });
//
//                const ChunkDistance chunk_distance {
//                    &(*chunk_col)[j],
//                    glm::distance2(chunk_world_pos, player_pos) };
//
//                // Enqueue chunk meshing
//                voxel::chunk_distance_queue_insert(m_chunks_to_mesh, chunk_distance);
//            }
//        }
    }
}