#pragma once

#include "core/types.h"

#include <string_view>
#include <typeindex>
#include <vector>

namespace h2o
{
    class Engine;
    struct GameInfo;

    class IModule
    {
    public:

        /**
         * Destructor, will be called even if the module fails to initialize.
         */
        virtual ~IModule() = default;

        /**
         * Initialize the module. Called just after the constructor.
         *
         * @param engine A reference to the engine. Modules can store a raw pointer to the engine
         *               if needed, which will be valid for their entire lifetime.
         * @return true if initialization was successful.
         */
        [[nodiscard]] virtual bool init(Engine& engine) { return true; }

        /**
         * Cleanup resources. Called just before the destructor.
         */
        virtual void cleanup() {}

        /**
         * @brief Getter for the module's name
         * 
         * @return The module's name
         */
        [[nodiscard]] virtual std::string_view module_name() const = 0;

        /**
         * @brief Get a list of dependencies that need to be initialized before this module can be initialized
         * 
         * @return A vector of module types
         */
        [[nodiscard]] virtual std::vector<std::type_index> dependencies() const
        { return {}; }

        [[nodiscard]] virtual std::vector<std::type_index> optional_dependencies() const
        { return {}; }
    };
}