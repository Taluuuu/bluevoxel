#pragma once

#include <string>
#include <typeindex>
#include <vector>

namespace engine
{
    class Module
    {
    public:

        ~Module() {}

        /**
         * @brief Init the module, called when all dependencies are loaded
         * 
         * @return true if the module was correctly initialized
         */
        virtual bool init();

        /**
         * @brief Free module resources
         * 
         */
        virtual void cleanup();

        /**
         * @brief Getter for the module's name
         * 
         * @return The module's name
         */
        virtual std::string get_module_name() const = 0;

        /**
         * @brief Get a list of dependencies before this module can be initialized
         * 
         * @return A vector of module types
         */
        virtual std::vector<std::type_index> get_dependencies() const = 0;

        /**
         * @brief Gets if the module has been initialized yet
         * 
         * @return true if the module has been initialized
         */
        bool is_initialized() const { return m_is_initialized; }

    private:

        bool m_is_initialized = false;

    };
}