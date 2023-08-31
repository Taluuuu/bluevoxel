#pragma once

#include "core/handle_types.h"
#include "core/types.h"

#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace h2o
{
    class ChunkColumn;

    class ChunkManager
    {
    public:

        

    private:

        std::unordered_map<v2i, OwningHandle<ChunkColumn>> m_loaded_chunks{};

    };
}