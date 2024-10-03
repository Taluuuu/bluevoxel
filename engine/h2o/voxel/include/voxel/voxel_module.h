#pragma once

#include "block.h"
#include "core/events.h"
#include "core/module.h"
#include "core/tickable.h"
#include "traits/block_trait_manager.h"

#include <memory>
#include <optional>
#include <vector>

namespace h2o
{
    class RenderingModule;
    class InventoryManager_Voxel;
    class VoxelPack;

    struct BlockModel;

    namespace gfx
    {
        class IPipeline;
        class TextureArray;
    }

    struct VoxelPackChangedEvent
    {
        const VoxelPack& voxel_pack;
    };

    class VoxelModule
        : public Tickable
        , public IModule
    {
    public:

        VoxelModule();
        ~VoxelModule() override = default;

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_voxel"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;
        [[nodiscard]] std::vector<std::type_index> optional_dependencies() const override;

        // Voxel pack
        [[nodiscard]] const std::shared_ptr<VoxelPack>& voxel_pack() const { return m_voxel_pack; }
        void set_voxel_pack(const std::shared_ptr<VoxelPack>& voxel_pack);

        [[nodiscard]] const BlockTraitManager& block_trait_manager() const { return m_block_trait_manager; }

        // Voxel rendering
        [[nodiscard]] std::optional<BlockModel> get_model(Block block, const std::vector<u32>*& out_texture_ids) const;
        [[nodiscard]] bool is_transparent(BlockID id) const;
        // Return value is always valid or an assert fails
        [[nodiscard]] const std::shared_ptr<gfx::IPipeline>& pipeline() const;
        [[nodiscard]] const std::shared_ptr<gfx::TextureArray>& block_textures() const;

        [[nodiscard]] std::shared_ptr<InventoryManager_Voxel> inventory_manager() const
        { return m_inventory_manager; }

    public:

        Event<VoxelPackChangedEvent> on_voxel_pack_changed{};

    private:

        [[nodiscard]] bool with_rendering() const
        { return m_rendering_module != nullptr; }

    private:

        BlockTraitManager m_block_trait_manager{};

        std::shared_ptr<VoxelPack> m_voxel_pack = nullptr;

        std::shared_ptr<InventoryManager_Voxel> m_inventory_manager = nullptr;

        // Rendering
        std::shared_ptr<gfx::IPipeline> m_pipeline = nullptr;
        std::shared_ptr<gfx::TextureArray> m_block_textures = nullptr;

        // Module refs
        RenderingModule* m_rendering_module = nullptr;

    };
}
