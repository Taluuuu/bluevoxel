#pragma once

#include "block_model.h"
#include "core/types.h"

#include <array>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace h2o
{
    class UncookedBlockModel
    {
    public:

        UncookedBlockModel() = default;

        [[nodiscard]] h2o::BlockModel build() const;

        [[nodiscard]] u32 face_count() const { return m_faces.size(); }

        struct Vertex
        {
            v3i position{};
            v2i uv{};

            [[nodiscard]] v3 world_pos() const
            { return v3{ position } / f32(h2o::voxel_constants::max_coord_value_per_block); }
        };

        using Triangle = std::array<Vertex, 3>;
        using Face = std::vector<Triangle>;

        struct FaceHandle
        {
            u32 face_index{};

            bool operator==(FaceHandle other) const
            { return face_index == other.face_index; }
        };

        struct TriangleHandle
        {
            FaceHandle face_handle{};
            u32 triangle_index{};

            bool operator==(TriangleHandle other) const
            { return face_handle == other.face_handle && triangle_index == other.triangle_index; }
        };

        struct VertexHandle
        {
            TriangleHandle triangle_handle{};
            u32 vertex_index{};

            bool operator==(const VertexHandle& other) const
            { return triangle_handle == other.triangle_handle && vertex_index == other.vertex_index; }
        };

        void add_face(const Face& face);

        void for_each_face(const std::function<void(FaceHandle, const Face&)>& function) const;
        void for_each_face(const std::function<void(FaceHandle, Face&)>& function);
        void for_each_triangle(const std::function<void(const TriangleHandle&, const Triangle&)>& function) const;
        void for_each_triangle(const std::function<void(const TriangleHandle&, Triangle&)>& function);
        void for_each_vertex(const std::function<void(const VertexHandle&, const Vertex&)>& function) const;
        void for_each_vertex(const std::function<void(const VertexHandle&, Vertex&)>& function);

        [[nodiscard]] const Face*     get_face(FaceHandle face_handle) const;
        [[nodiscard]]       Face*     get_face(FaceHandle face_handle);
        [[nodiscard]] const Triangle* get_triangle(const TriangleHandle& triangle_handle) const;
        [[nodiscard]]       Triangle* get_triangle(const TriangleHandle& triangle_handle);
        [[nodiscard]] const Vertex*   get_vertex(const VertexHandle& vertex_handle) const;
        [[nodiscard]]       Vertex*   get_vertex(const VertexHandle& vertex_handle);

    public:

        u32 id{};
        std::string name{};

    private:

        std::vector<Face> m_faces{};

    };
}

namespace YAML
{
    using namespace h2o;

    template<>
    struct convert<UncookedBlockModel>
    {
        static Node encode(const UncookedBlockModel& rhs);
        static bool decode(const Node& node, UncookedBlockModel& rhs);
    };

    template<>
    struct convert<UncookedBlockModel::Vertex>
    {
        static Node encode(const UncookedBlockModel::Vertex& rhs);
        static bool decode(const Node& node, UncookedBlockModel::Vertex& rhs);
    };
}