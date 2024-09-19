#include "inventory/inventory_module.h"

#include "inventory/inventory.h"
#include "inventory/inventory_ui.h"
#include "inventory/hotbar_ui.h"
#include "scene/scene_module.h"
#include "ui/ui_module.h"

namespace h2o
{
    bool InventoryModule::init(Engine& engine)
    {
        return true;
    }

    std::vector<std::type_index> InventoryModule::dependencies() const
    {
        return { typeid(SceneModule), typeid(UIModule) };
    }
}