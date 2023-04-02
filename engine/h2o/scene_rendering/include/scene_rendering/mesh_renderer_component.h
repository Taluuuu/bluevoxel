#pragma once

#include "scene/component.h"

namespace h2o
{
    namespace gfx { class IVertexArray; }

    class MeshRendererComponent : public Component
    {
    public:

        explicit MeshRendererComponent(const ComponentInitializer& component_initializer);
        ~MeshRendererComponent() override;

    public:

        // This will need a better abstraction down the line, like a Mesh class
        std::shared_ptr<gfx::IVertexArray> vao = nullptr;

    };
}