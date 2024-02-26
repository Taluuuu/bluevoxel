#pragma once

#include "core/types.h"
#include "voxel/uncooked_block_model.h"

#include <optional>
#include <unordered_set>
#include <variant>

namespace bluevoxel
{
    using VertexSelection = std::unordered_set<v3i>;
    using FaceSelection = std::optional<h2o::UncookedBlockModel::FaceHandle>;
    struct TriangleSelection
    {
        std::optional<h2o::UncookedBlockModel::TriangleHandle> triangle_handle{};
        std::unordered_set<u32> vertex_indices{};
    };

    using SelectionTypes = std::variant<FaceSelection, TriangleSelection, VertexSelection>;
}