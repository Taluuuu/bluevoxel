#include "inventory/inventory_module.h"

#include "inventory/inventory.h"
#include "inventory/inventory_ui.h"

namespace h2o
{
    bool InventoryModule::init(Engine& engine)
    {
        return true;
    }

    std::vector<std::type_index> InventoryModule::dependencies() const
    {
        return {};
    }
}