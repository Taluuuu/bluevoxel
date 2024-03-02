#pragma once

#include "core/types.h"
#include "voxel/uncooked_block_model.h"

#include <optional>
#include <unordered_set>
#include <variant>

namespace bluevoxel
{
    using FaceSelection = std::optional<h2o::UncookedBlockModel::FaceHandle>;

    using SelectionTypes = std::variant<FaceSelection>;
}