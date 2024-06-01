#pragma once

#include "inventory/inventory_ui.h"
#include "voxel/block.h"

#include <optional>

namespace h2o
{
    class VoxelModule;
}

namespace bluevoxel
{
    class BluevoxelInventoryUI : public h2o::InventoryUI<h2o::Block>
    {
    public:

        explicit BluevoxelInventoryUI(h2o::Tickable* owner);
        ~BluevoxelInventoryUI() override = default;

    protected:

        [[nodiscard]] std::optional<u32> fetch_item_texture_id(const h2o::Block& item) const override;
        [[nodiscard]] std::string fetch_item_name(const h2o::Block& item) const override;

    private:

        h2o::VoxelModule* const m_voxel_module = nullptr;

    };
}