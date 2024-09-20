#pragma once

#include "inventory/inventory_draw_data.h"
#include "voxel/block.h"

namespace h2o
{
    class VoxelModule;
}

namespace h2o
{
    class VoxelInventoryDrawData : public InventoryDrawData<Block>
    {
    public:

        VoxelInventoryDrawData();
        explicit VoxelInventoryDrawData(const VoxelModule& voxel_module);
        ~VoxelInventoryDrawData() override = default;

        // InventoryUI interface
        [[nodiscard]] std::optional<u32> fetch_item_texture_id(const Block& item) const override;
        [[nodiscard]] std::string fetch_item_name(const Block& item) const override;

    private:

        const VoxelModule* const m_voxel_module = nullptr;

    };
}