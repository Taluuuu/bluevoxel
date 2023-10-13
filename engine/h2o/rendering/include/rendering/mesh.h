#pragma once

#include "core/types.h"
#include "core/resources.h"

namespace h2o::gfx
{
    class IVertexArray;

    class Mesh : public IResource
    {
    public:

        Mesh() = default;
        Mesh(const Mesh&) = delete;
        Mesh(Mesh&&) = delete;
        ~Mesh() override = default;

        [[nodiscard]] const std::shared_ptr<IVertexArray>& vertex_array() const { return m_vertex_array; }
        [[nodiscard]] i32 vertex_count() const { return m_vertex_count; }

        // IResource interface
        bool load(const std::string& path) override;

    private:

        std::shared_ptr<IVertexArray> m_vertex_array = nullptr;

        i32 m_vertex_count = 0;

    };
}