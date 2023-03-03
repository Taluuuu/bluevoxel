#pragma once

#include <string_view>
#include <typeindex>
#include <vector>

namespace h2o
{
    class Engine;
    struct GameInfo;

    class Module
    {
    public:

        explicit Module(Engine& engine);
        virtual ~Module() = default;

        /**
         * @brief Try initializing the module
         * 
         * @param game_info The struct containing the game's info
         * @return true if the module was correctly initialized, false otherwise.
         */
        virtual bool init(const GameInfo& game_info) { return true; }

        /**
         * @brief Free module resources
         * 
         */
        virtual void cleanup() {}

        /**
         * @brief Getter for the module's name
         * 
         * @return The module's name
         */
        [[nodiscard]] virtual std::string_view module_name() const = 0;

        /**
         * @brief Get a list of dependencies before this module can be initialized
         * 
         * @return A vector of module types
         */
        [[nodiscard]] virtual std::vector<std::type_index> dependencies() const = 0;

    protected:

        Engine* const m_engine = nullptr;

    };
}