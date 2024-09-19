#pragma once

#include "inventory/inventory_draw_data.h"
#include "voxel/block.h"

namespace h2o
{
    class VoxelModule;
}

namespace bluevoxel
{
    class BluevoxelInventoryDrawData : public h2o::InventoryDrawData<h2o::Block>
    {
    public:

        BluevoxelInventoryDrawData();
        ~BluevoxelInventoryDrawData() override = default;

        // h2o::InventoryUI interface
        [[nodiscard]] std::optional<u32> fetch_item_texture_id(const h2o::Block& item) const override;
        [[nodiscard]] std::string fetch_item_name(const h2o::Block& item) const override;

    private:

        h2o::VoxelModule* const m_voxel_module = nullptr;

    };
}