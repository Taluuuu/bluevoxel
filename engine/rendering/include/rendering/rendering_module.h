#pragma once

#include "core/module.h"

#include <memory>

namespace engine
{
    class IRenderer;

    class RenderingModule : public Module<RenderingModule>
    {
    public:

        static std::shared_ptr<IRenderer> create_renderer();

    protected:

        friend class Module<RenderingModule>;

        static bool init_impl();
        static void cleanup_impl();

    };
}