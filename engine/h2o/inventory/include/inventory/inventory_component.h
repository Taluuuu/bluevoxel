#pragma once

#include "inventory.h"
#include "scene/component.h"

namespace h2o
{
    template<class ItemType>
    class InventoryComponent : public Component
    {
    public:

        using InventoryPtr = std::shared_ptr< Inventory<ItemType> >;

        InventoryComponent(
            const ComponentInitializer& component_initializer,
            const InventoryPtr& inventory);

        [[nodiscard]] const InventoryPtr& inventory() const { return m_inventory; }

    private:

        InventoryPtr m_inventory = nullptr;

    };

    template<class ItemType>
    InventoryComponent<ItemType>::InventoryComponent(
        const ComponentInitializer& component_initializer,
        const InventoryPtr& inventory)
        : Component(component_initializer)
        , m_inventory(inventory)
    {}
}