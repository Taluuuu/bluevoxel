#pragma once

#include "inventory/hotbar_ui.h"
#include "scene/component.h"
#include "voxel/block.h"

namespace h2o
{
    class CameraComponent;
    class ChunkManager;
    class IBlockPlaceable;
    class InputComponent;
    class RenderingModule;

    class BlockPlacingComponent : public Component
    {
    public:

        explicit BlockPlacingComponent(const ComponentInitializer& component_initializer);
        ~BlockPlacingComponent() override = default;

        void update(f32 delta_time) override;

    public:

        WeakHandle<IBlockPlaceable> block_placeable = nullptr;

    private:

        WeakHandle<CameraComponent> m_camera = nullptr;
        WeakHandle<InputComponent> m_input = nullptr;

        std::shared_ptr< Inventory<Block> > m_hotbar_inventory = nullptr;
        HotbarUI<Block> m_hotbar_ui;

        // Module refs
        RenderingModule* const m_rendering_module = nullptr;

    };
}
