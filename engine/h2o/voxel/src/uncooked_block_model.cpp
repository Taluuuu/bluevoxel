#include "voxel/uncooked_block_model.h"

#include "voxel/voxel_constants.h"
#include "voxel/voxel_pack.h"

namespace h2o
{
    static BlockModel::Triangle process_triangle(const UncookedBlockModel::Triangle& unprocessed_triangle, u32 face_index)
    {
        BlockModel::Triangle triangle{};

        const auto& v0 = unprocessed_triangle.vertices[0];
        const auto& v1 = unprocessed_triangle.vertices[1];
        const auto& v2 = unprocessed_triangle.vertices[2];

        const v3i& p0 = v0.position;
        const v3i& p1 = v1.position;
        const v3i& p2 = v2.position;

        // Pack normal vector
        const v3 p0_to_p1 = p1 - p0;
        const v3 p0_to_p2 = p2 - p0;
        const v3 normal = glm::normalize(glm::cross(p0_to_p1, p0_to_p2));
        const f32 n_pitch = std::asin(normal.y);
        const f32 n_yaw = std::atan2(normal.x, normal.z);

        // Remap from 0 to 1 to integer values depending on their number of bits
        const f32 normalized_pitch = (n_pitch + glm::half_pi<f32>()) / glm::pi<f32>();
        const f32 normalized_yaw = (n_yaw + glm::pi<f32>()) / glm::two_pi<f32>();

        const u32 packed_n_pitch = std::lround(normalized_pitch * voxel_constants::packed_pitch_max_value);
        const u32 packed_n_yaw = std::lround(normalized_yaw * voxel_constants::packed_yaw_max_value);

        for (i32 vertex_index = 0; vertex_index < 3; vertex_index++)
        {
            const auto& vertex = unprocessed_triangle.vertices[vertex_index];
            triangle[vertex_index] =
                BlockVertex
                {
                    .x = u32(vertex.position.x), // TODO: Make sure the values are in an ok range
                    .y = u32(vertex.position.y),
                    .z = u32(vertex.position.z),
                    .u = u32(vertex.uv.x),
                    .v = u32(vertex.uv.y),
                    .tex_idx = face_index,
                    .n_pitch = packed_n_pitch,
                    .n_yaw = packed_n_yaw,
                };
        }

        return triangle;
    }

    h2o::BlockModel UncookedBlockModel::build() const
    {
        h2o::BlockModel result{};

        u32 face_index = 0;
        for (const auto& face : m_faces)
        {
            for (const auto& triangle : face)
            {
                if (triangle.is_hidden)
                    continue;

                using namespace h2o;

                voxel::Direction::Type triangle_occluding_directions = voxel::Direction::All;

                // Find which faces need to be occluded by which axis
                i32 axis_index = 0;
                magic_enum::enum_for_each<voxel::Axis>([&](voxel::Axis axis)
                {
                    const auto dir = voxel::to_direction(axis);
                    const auto inv_dir = voxel::invert(dir);

                    for (const auto& vertex : triangle.vertices)
                    {
                        if (vertex.position[axis_index] > 0)
                        {
                            // Triangle can't be occluded by neighboring block at inv_dir
                            triangle_occluding_directions = static_cast<voxel::Direction::Type>(
                                triangle_occluding_directions & ~inv_dir);
                        }

                        if (vertex.position[axis_index] < voxel_constants::max_coord_value_per_block)
                        {
                            // Triangle can't be occluded by neighboring block at dir
                            triangle_occluding_directions = static_cast<voxel::Direction::Type>(
                                triangle_occluding_directions & ~dir);
                        }
                    }

                    axis_index++;
                });

                if (triangle_occluding_directions)
                {
                    for (i32 i = 0; i < 6; i++)
                    {
                        if (triangle_occluding_directions & (1 << i))
                        {
                            const auto processed_triangle = process_triangle(triangle, face_index);
                            result.occluded_triangles_per_side[i].push_back(processed_triangle);
                            break;
                        }
                    }
                }
                else
                {
                    // The triangle does not need to be occluded
                    const auto processed_triangle = process_triangle(triangle, face_index);
                    result.unoccluded_triangles.push_back(processed_triangle);
                }
            }

            face_index++;
        }

        return result;
    }

    void UncookedBlockModel::add_face(const UncookedBlockModel::Face& face)
    {
        m_faces.push_back(face);
    }

    void UncookedBlockModel::for_each_face(const std::function<void(FaceHandle, const Face&)>& function) const
    {
        u32 face_index = 0;
        for (const auto& face : m_faces)
        {
            function(FaceHandle{ face_index }, face);
            face_index++;
        }
    }

    void UncookedBlockModel::for_each_face(const std::function<void(FaceHandle, Face&)>& function)
    {
        u32 face_index = 0;
        for (auto& face : m_faces)
        {
            function(FaceHandle{ face_index }, face);
            face_index++;
        }
    }

    void UncookedBlockModel::for_each_triangle(const std::function<void(const TriangleHandle&, const Triangle&)>& function) const
    {
        u32 face_index = 0;
        for (const auto& face : m_faces)
        {
            u32 triangle_index = 0;
            for (const auto& triangle : face)
            {
                function(TriangleHandle{ face_index, triangle_index }, triangle);
                triangle_index++;
            }

            face_index++;
        }
    }

    void UncookedBlockModel::for_each_triangle(const std::function<void(const TriangleHandle&, Triangle&)>& function)
    {
        u32 face_index = 0;
        for (auto& face : m_faces)
        {
            u32 triangle_index = 0;
            for (auto& triangle : face)
            {
                function(TriangleHandle{ face_index, triangle_index }, triangle);
                triangle_index++;
            }

            face_index++;
        }
    }

    void UncookedBlockModel::for_each_vertex(const std::function<void(const VertexHandle&, const Vertex&)>& function) const
    {
        u32 face_index = 0;
        for (const auto& face : m_faces)
        {
            u32 triangle_index = 0;
            for (const auto& triangle : face)
            {
                for (u32 i = 0; i < 3; i++)
                    function(VertexHandle{ face_index, triangle_index, i }, triangle.vertices[i]);

                triangle_index++;
            }

            face_index++;
        }
    }

    void UncookedBlockModel::for_each_vertex(const std::function<void(const VertexHandle&, Vertex&)>& function)
    {
        u32 face_index = 0;
        for (auto& face : m_faces)
        {
            u32 triangle_index = 0;
            for (auto& triangle : face)
            {
                for (u32 i = 0; i < 3; i++)
                    function(VertexHandle{ face_index, triangle_index, i }, triangle.vertices[i]);

                triangle_index++;
            }

            face_index++;
        }
    }

    const UncookedBlockModel::Face* UncookedBlockModel::get_face(FaceHandle face_handle) const
    {
        if (face_handle.face_index < m_faces.size())
            return &m_faces[face_handle.face_index];

        return nullptr;
    }

    UncookedBlockModel::Face* UncookedBlockModel::get_face(FaceHandle face_handle)
    {
        if (face_handle.face_index < m_faces.size())
            return &m_faces[face_handle.face_index];

        return nullptr;
    }

    const UncookedBlockModel::Triangle* UncookedBlockModel::get_triangle(const TriangleHandle& triangle_handle) const
    {
        const auto [face_handle, triangle_index] = triangle_handle;
        if (const auto face = get_face(face_handle))
        {
            if (triangle_index < face->size())
                return &(*face)[triangle_index];
        }

        return nullptr;
    }

    UncookedBlockModel::Triangle* UncookedBlockModel::get_triangle(const TriangleHandle& triangle_handle)
    {
        const auto [face_handle, triangle_index] = triangle_handle;
        if (const auto face = get_face(face_handle))
        {
            if (triangle_index < face->size())
                return &(*face)[triangle_index];
        }

        return nullptr;
    }

    const UncookedBlockModel::Vertex* UncookedBlockModel::get_vertex(const VertexHandle& vertex_handle) const
    {
        const auto [triangle_handle, vertex_index] = vertex_handle;
        if (const auto triangle = get_triangle(triangle_handle))
        {
            if (vertex_index < triangle->vertices.size())
                return &triangle->vertices[vertex_index];
        }

        return nullptr;
    }

    UncookedBlockModel::Vertex* UncookedBlockModel::get_vertex(const VertexHandle& vertex_handle)
    {
        const auto [triangle_handle, vertex_index] = vertex_handle;
        if (auto triangle = get_triangle(triangle_handle))
        {
            if (vertex_index < triangle->vertices.size())
                return &triangle->vertices[vertex_index];
        }

        return nullptr;
    }
}

namespace YAML
{
    Node convert<UncookedBlockModel>::encode(const UncookedBlockModel& rhs)
    {
        // TODO :))
        return Node{};
    }

    bool convert<UncookedBlockModel>::decode(const Node& node, UncookedBlockModel& model)
    {
        const auto model_name = node["name"].as<std::string>();
        model = UncookedBlockModel{};
        model.name = model_name;

        for (const auto face_node: node["faces"])
        {
            const auto unprocessed_face = face_node["triangles"].as<UncookedBlockModel::Face>();
            model.add_face(unprocessed_face);
        }

        return true;
    }

    Node convert<UncookedBlockModel::Vertex>::encode(const UncookedBlockModel::Vertex& vertex)
    {
        Node node{};
        node.push_back(vertex.position.x);
        node.push_back(vertex.position.y);
        node.push_back(vertex.position.z);

        return node;
    }

    bool convert<UncookedBlockModel::Vertex>::decode(const Node& node, UncookedBlockModel::Vertex& rhs)
    {
        if (!node.IsSequence() || node.size() != 5)
            return false;

        rhs.position.x = node[0].as<i32>();
        rhs.position.y = node[1].as<i32>();
        rhs.position.z = node[2].as<i32>();
        rhs.uv.x = node[3].as<i32>();
        rhs.uv.y = node[4].as<i32>();

        return true;
    }

    Node convert<UncookedBlockModel::Triangle>::encode(const UncookedBlockModel::Triangle& rhs)
    {
        // TODO :))
        return Node{};
    }

    bool convert<UncookedBlockModel::Triangle>::decode(const Node& node, UncookedBlockModel::Triangle& rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;

        rhs.vertices[0] = node[0].as<UncookedBlockModel::Vertex>();
        rhs.vertices[1] = node[1].as<UncookedBlockModel::Vertex>();
        rhs.vertices[2] = node[2].as<UncookedBlockModel::Vertex>();
        rhs.is_hidden = false;

        return true;
    }
}