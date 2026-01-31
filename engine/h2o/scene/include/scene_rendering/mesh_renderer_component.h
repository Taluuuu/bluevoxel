#pragma once

#include <memory>
#include <string>

namespace h2o
{
    namespace gfx
    {
        class Texture;
        class Mesh;
    }

    struct MeshRenderer
    {
        // Serialized
        std::string mesh_path{};
        std::string texture_path{};

        // Runtime only
        // TODO: Split mesh and texture into separate components?
        std::shared_ptr<gfx::Texture> texture = nullptr;
        std::shared_ptr<gfx::Mesh> mesh = nullptr;

        template<typename S>
        void serialize(S& s)
        { s(mesh_path, texture_path); }
    };

    struct MeshNode
    {
        // Serialized
        std::string node_name{};

        // TODO: Change this to a mesh node or smt
        std::shared_ptr<gfx::Mesh> mesh = nullptr;

        template<typename S>
        void serialize(S& s)
        { s(node_name); }
    };
}