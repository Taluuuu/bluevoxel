#pragma once

#include "block_model.h"
#include "core/types.h"

#include <array>
#include <vector>

namespace h2o
{
    class UncookedBlockModel
    {
    public:

        explicit UncookedBlockModel(const std::string& name, u32 id);

        using Vertex = std::array<u32, 5>; // x, y, z, u, v
        using Triangle = std::vector<Vertex>;
        using Face = std::vector<Triangle>;

        [[nodiscard]] h2o::BlockModel build() const;

        [[nodiscard]] u32 face_count() const { return m_faces.size(); }

        [[nodiscard]] const std::string& name() const { return m_name; }
        [[nodiscard]] u32 id() const { return m_id; }

        void add_face(const Face& face);

    private:

        std::string m_name{};
        u32 m_id{};

        std::vector<Face> m_faces{};

    };
}