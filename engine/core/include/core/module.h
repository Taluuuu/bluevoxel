#pragma once

#include <string_view>
#include <typeindex>
#include <vector>

namespace engine
{
    class Engine;
    struct GameInfo;

    class Module
    {
    public:

        Module(Engine& engine);
        virtual ~Module() {}

        /**
         * @brief Try initializing the module
         * 
         * @param game_info The struct containing the game's info
         * @return true if the module was correctly initialized, false otherwise.
         */
        virtual bool init(const GameInfo& game_info);

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
        virtual std::string_view get_module_name() const = 0;

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

        /**
         * @brief Get the engine instance this module is owned by
         * 
         * @return A reference to the engine
         */
        Engine& engine() const { return m_engine; }

    private:

        bool m_is_initialized = false;

        Engine& m_engine;

    };
}