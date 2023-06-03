#pragma once

#include "core/log.h"

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

namespace h2o
{
    class IResource
    {
    public:

        virtual bool load(const std::string& path) = 0;

    };

    class ResourceManager
    {
    public:

        /**
         * Get a resource handle by its path. Load it from disk if it was not
         * previously loaded.
         */
        template<class T>
        requires (std::derived_from<T, IResource> && !std::same_as<IResource, T>)
        std::shared_ptr<T> fetch(const std::string& path);

        /**
         * Stop owning loaded resources. Shared pointers to resources stored
         * elsewhere will stay valid.
         */
        void unload_all();

    private:

        using ResourceHandle = std::shared_ptr<IResource>;
        std::unordered_map<std::string, ResourceHandle> m_resources;

    };

    template<class T>
    requires (std::derived_from<T, IResource> && !std::same_as<IResource, T>)
    std::shared_ptr<T> ResourceManager::fetch(const std::string& path)
    {
        if (auto res_it = m_resources.find(path); res_it != m_resources.end())
        {
            assert(res_it->second);

            auto res = std::dynamic_pointer_cast<T>(res_it->second);
            assert(res); // Is the resource the right type ?

            return res;
        }

        auto res = std::make_shared<T>();
        if (!res || !res->load(path))
        {
            log::warn("Failed to load resource at path: '{}'", path);
            return nullptr;
        }

        m_resources[path] = res;
        return res;
    }
}