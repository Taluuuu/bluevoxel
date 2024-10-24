#pragma once

#include "inventory/inventory.h"
#include "scene/component.h"
#include "voxel/block.h"

namespace h2o
{
    namespace gfx
    {
        class Texture;
    }

    class CameraComponent;
    class ChunkManager;
    class IBlockPlaceable;
    class InputComponent;

    class RenderingModule;
    class VoxelModule;

    class BlockPlacingComponent : public Component
    {
    public:

        explicit BlockPlacingComponent(const ComponentInitializer& component_initializer);
        ~BlockPlacingComponent() override = default;

        void post_update(f32 delta_time) override;

    public:

        WeakHandle<IBlockPlaceable> block_placeable = nullptr;

    private:

        WeakHandle<CameraComponent> m_camera = nullptr;
        WeakHandle<InputComponent> m_input = nullptr;

        std::shared_ptr< Inventory<Block> > m_inventory = nullptr;
        std::shared_ptr< Inventory<Block> > m_hotbar_inventory = nullptr;

        v2 m_crosshair_size{ 16.0f };
        std::shared_ptr<gfx::Texture> m_crosshair_texture = nullptr;

        // Module refs
        RenderingModule* const m_rendering_module = nullptr;
        VoxelModule* const m_voxel_module = nullptr;

    };
}
