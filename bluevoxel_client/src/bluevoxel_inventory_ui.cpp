#include "bluevoxel_inventory_ui.h"

#include "core/engine.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"

namespace bluevoxel
{
    BluevoxelInventoryUI::BluevoxelInventoryUI(h2o::Tickable* owner)
        : InventoryUI(owner)
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
    {}

    std::optional<u32> BluevoxelInventoryUI::fetch_item_texture_id(const h2o::Block& item) const
    {
        if (const auto voxel_pack = m_voxel_module->voxel_pack())
        {
            if (const auto block_type = voxel_pack->get_block_type(item.id))
            {
                const auto& texture_ids = block_type->texture_ids;
                if (!texture_ids.empty())
                    return texture_ids[glm::min(1, i32(texture_ids.size()) - 1)];
            }
        }

        return std::nullopt;
    }

    std::string BluevoxelInventoryUI::fetch_item_name(const h2o::Block& item) const
    {
        if (const auto voxel_pack = m_voxel_module->voxel_pack())
        {
            if (const auto block_type = voxel_pack->get_block_type(item.id))
                return block_type->name;
        }

        return "";
    }
}