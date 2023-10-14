#pragma once

#include "core/types.h"

namespace h2o
{
    class Transform
    {
    public:

        explicit Transform(
            const v3& position = v3 { 0.0f, 0.0f, 0.0f },
            const v3& rotation = v3 { 0.0f, 0.0f, 0.0f },
            const v3& scale = v3 { 1.0f, 1.0f, 1.0f });

        ~Transform() = default;

        /**
         * Construct a model matrix from this transform's data.
         * Result is not cached, avoid calling this multiple times per frame if possible.
         *
         * @return The constructed model matrix
         */
        [[nodiscard]] m4 model_matrix() const;

        template<typename S>
        void serialize(S& s)
        { s(position, rotation, scale); }

    public:

        v3 position { 0.0f, 0.0f, 0.0f };
        v3 rotation { 0.0f, 0.0f, 0.0f };
        v3 scale    { 1.0f, 1.0f, 1.0f };

    };
}