#pragma once

#include "core/types.h"
#include "scene/component.h"

namespace h2o
{
    class TransformComponent : public Component
    {
    public:

        explicit TransformComponent(const ComponentInitializer& component_initializer);

        /**
         * Construct a model matrix from this transform's data.
         * Result is not cached, avoid calling this multiple times per frame if possible.
         *
         * @return The constructed model matrix
         */
        [[nodiscard]] m4 model_matrix() const;

    public:

        v3 position { 0.0f, 0.0f, 0.0f };
        v3 rotation { 0.0f, 0.0f, 0.0f };
        v3 scale    { 1.0f, 1.0f, 1.0f };

    };
}