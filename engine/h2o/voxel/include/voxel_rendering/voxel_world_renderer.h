#pragma once

#include "core/tickable.h"
#include "rendering/vertex_array.h"

namespace h2o
{
    class ChunkManager;

    class VoxelWorldRenderer : public Tickable
    {

    public:

        VoxelWorldRenderer(Tickable& owner, const ChunkManager& chunk_manager);

    protected:

        // Tickable interface
        void render() override;

    private:

        // struct ChunkMeshData
        // {
        //     gfx::VertexArray vertex_array;
        //     u32 vertex_count = 0;
        //     v3i chunk_pos{};
        // };
        //
        // std::vector<ChunkMeshData> m_chunk_mesh_pool{};
        // std::unordered_map<v3i, u32> m_chunk_mesh_indices{};

        const ChunkManager* m_chunk_manager = nullptr;

    };
}
