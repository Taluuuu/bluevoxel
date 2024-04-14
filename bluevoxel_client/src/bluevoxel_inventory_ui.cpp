#include "bluevoxel_inventory_ui.h"

#include "core/engine.h"
#include "rendering/texture.h"
#include "rendering/texture_array.h"
#include "voxel/voxel_module.h"
#include "voxel/voxel_pack.h"
#include "voxel_rendering/voxel_rendering_module.h"

namespace bluevoxel
{
    BluevoxelInventoryUI::BluevoxelInventoryUI(h2o::Tickable* owner)
        : InventoryUI(owner)
        , m_voxel_module(&g_engine->get_module_checked<h2o::VoxelModule>())
        , m_voxel_rendering_module(&g_engine->get_module_checked<h2o::VoxelRenderingModule>())
    {}

    std::optional<u32> BluevoxelInventoryUI::fetch_item_texture_id(const h2o::Block& item) const
    {
        const auto voxel_pack = m_voxel_module->voxel_pack();
        if (!voxel_pack)
            return std::nullopt;

        const auto block_type = voxel_pack->get_block_type(item.id);
        if (!block_type)
            return std::nullopt;

        const auto& texture_ids = block_type->texture_ids;
        if (texture_ids.empty())
            return std::nullopt;

        // Use either the second or the first face's texture
        const i32 face_index = glm::min(1, i32(texture_ids.size()) - 1);
        if (face_index < 0)
            return std::nullopt;

        const u32 texture_id = texture_ids[face_index];
        const auto& texture_array = m_voxel_rendering_module->block_textures();
        if (!texture_array)
            return std::nullopt;

        const auto& block_textures = texture_array->textures();
        if (texture_id < block_textures.size())
        {
            if (const auto& texture = block_textures[texture_id])
                return texture->id();
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