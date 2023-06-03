#include "core/resources.h"

namespace h2o
{
    void ResourceManager::unload_all()
    {
        m_resources.clear();
    }
}