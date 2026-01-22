#include "game_framework/game_framework_module.h"

#include "rendering/rendering_module.h"
#include "scene/scene_module.h"

namespace h2o
{
    std::vector<std::type_index> GameFrameworkModule::dependencies() const
    {
        return { typeid(SceneModule), typeid(RenderingModule) };
    }
}
