#pragma once

#include "core/types.h"

namespace h2o
{
    struct Transform
    {
    public:

        v3 position { 0.0f, 0.0f, 0.0f };
        v3 rotation { 0.0f, 0.0f, 0.0f };
        v3 scale    { 1.0f, 1.0f, 1.0f };

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

    };
}