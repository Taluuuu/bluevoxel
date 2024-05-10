#pragma once

#include "core/log.h"

#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace fs = std::filesystem;

namespace h2o
{
    class IResource
    {
    public:

        virtual ~IResource() = default;

        virtual bool load(const fs::path& path) = 0;

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
         * Get a resource handle by its path. Always load from disk.
         */
        template<class T>
        requires (std::derived_from<T, IResource> && !std::same_as<IResource, T>)
        std::shared_ptr<T> reload(const std::string& path);

        /**
         * Stop owning loaded resources. Shared pointers to resources stored
         * elsewhere will stay valid.
         */
        void unload_all();

    private:

        static constexpr std::string base_resource_path = "../Resources/";

        using ResourceHandle = std::shared_ptr<IResource>;
        std::unordered_map<fs::path, ResourceHandle> m_resources;

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

        return reload<T>(path);
    }

    template<class T>
    requires (std::derived_from<T, IResource> && !std::same_as<IResource, T>)
    std::shared_ptr<T> ResourceManager::reload(const std::string& path)
    {
        auto res = std::make_shared<T>();
        if (!res || !res->load(fs::path(base_resource_path) / path))
        {
            log::warn("Failed to load resource at path: '{}'", path);
            return nullptr;
        }

        m_resources[path] = res;
        return res;
    }
}