#pragma once

#include "core/engine.h"
#include "core/module.h"

#include <memory>
#include <typeindex>
#include <unordered_map>

// Forward declarations
namespace engine
{
    class IWindow;
}

class Application
{
public:

    void run();

    template<typename T>
    Application& add_module()
    {
        std::type_index module_type(std::type_info(T));
        return *this;
    }

private:

    bool init();
    void update();
    void cleanup();

private:

    engine::Engine m_engine;

    std::shared_ptr<engine::IWindow> m_window = nullptr;

};