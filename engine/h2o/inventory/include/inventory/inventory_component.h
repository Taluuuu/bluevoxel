#pragma once

#include "inventory.h"
#include "scene/component.h"

namespace h2o
{
    template<class ItemType>
    class InventoryComponent : public Component
    {
    public:

        InventoryComponent(
            const ComponentInitializer& component_initializer,
            const Inventory<ItemType>& inventory);

        [[nodiscard]] const Inventory<ItemType>& inventory() const { return m_inventory; }
        [[nodiscard]]       Inventory<ItemType>& inventory()       { return m_inventory; }

    private:

        Inventory<ItemType> m_inventory;

    };

    template<class ItemType>
    InventoryComponent<ItemType>::InventoryComponent(
        const ComponentInitializer& component_initializer,
        const Inventory<ItemType>& inventory)
        : Component(component_initializer)
        , m_inventory(inventory)
    {}
}