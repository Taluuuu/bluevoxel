#include "scene_rendering/scene_rendering_module.h"

#include "rendering/rendering_module.h"
#include "scene/scene_module.h"

namespace h2o
{
    std::vector<std::type_index> SceneRenderingModule::dependencies() const
    {
        return { typeid(RenderingModule), typeid(SceneModule) };
    }
}