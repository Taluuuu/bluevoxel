#pragma once

#include "core/resources.h"
#include "core/types.h"

#include <assimp/scene.h>
#include <span>
#include <string>
#include <vector>

namespace h2o
{
    namespace gfx
    {
        class IRenderer;
        class VertexArray;
    }

    class MeshScene : public IResource
    {
    public:

        MeshScene();

        // IResource interface
        bool load(const fs::path& path) override;

        struct Mesh
        {
            std::shared_ptr<gfx::VertexArray> vertex_array;
            i32 vertex_count;
        };

        struct Node
        {
            std::string name{};

            v3 position{}, rotation{}, scale{};
            Mesh* mesh = nullptr;

            std::vector<Node> children{};
        };

    private:

        Node visit_node(const aiNode& assimp_node);
        void create_meshes(std::span<aiMesh*> meshes);

    private:

        Node m_root_node{};
        std::vector<Mesh> m_meshes{};

        gfx::IRenderer* m_renderer = nullptr;

    };
}
