#pragma once

#include "core/types.h"

#include <memory>
#include <optional>
#include <string>

namespace h2o
{
    template<class ItemType>
    class InventoryDrawData
    {
    public:

        virtual ~InventoryDrawData() = default;

        [[nodiscard]] virtual std::optional<u32> fetch_item_texture_id(const ItemType& item) const = 0;
        [[nodiscard]] virtual std::string fetch_item_name(const ItemType& item) const = 0;

    public:

        f32 item_texture_size = 64.0f;
        f32 item_texture_spacing = 8.0f;

    };
}