#pragma once

#include <memory>

// Forward declarations
namespace engine
{
    class IWindow;
}

class Application
{
public:

    void run();

private:

    bool init();
    void update();
    void cleanup();

private:

    std::shared_ptr<engine::IWindow> m_window = nullptr;

};