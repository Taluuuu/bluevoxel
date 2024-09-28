#pragma once

#include "inventory/inventory_manager_base.h"
#include "voxel/block.h"

namespace h2o
{
    class VoxelModule;
}

namespace h2o
{
    class InventoryManager_Voxel : public InventoryManager_Base<Block>
    {
    public:

        explicit InventoryManager_Voxel(VoxelModule& voxel_module);
        ~InventoryManager_Voxel() override = default;

        // InventoryUI interface
        [[nodiscard]] std::optional<u32> fetch_item_texture_id(const Block& item) const override;
        [[nodiscard]] std::string fetch_item_name(const Block& item) const override;

    private:

        const VoxelModule* const m_voxel_module = nullptr;

    };
}