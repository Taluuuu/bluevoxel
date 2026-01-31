#include "rendering/mesh_scene.h"

#include "core/engine.h"
#include "rendering/buffer.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/vertex_array.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace h2o
{
    MeshScene::MeshScene()
    {
        if (const auto rendering_module = g_engine->get_module<RenderingModule>())
            m_renderer = &rendering_module->renderer();
    }

    bool MeshScene::load(const fs::path& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path.string(),
            aiProcess_Triangulate      |
            aiProcess_JoinIdenticalVertices   |
            aiProcess_FlipUVs                 |
            aiProcess_GlobalScale);

        if (!scene || !scene->mRootNode)
        {
            log::error("Failed to import mesh at '{}': {}.", path.string(), importer.GetErrorString());
            return false;
        }

        create_meshes({ scene->mMeshes, scene->mNumMeshes });
        m_root_node = visit_node(*scene->mRootNode);

        importer.FreeScene();

        return true;
    }

    MeshScene::Node MeshScene::visit_node(const aiNode& assimp_node)
    {
        Node node{ assimp_node.mName.C_Str() };

        aiVector3f position, rotation, scale;
        assimp_node.mTransformation.Decompose(scale, rotation, position);
        node.position = v3{ position.x, position.y, position.z };
        node.rotation = v3{ rotation.x, rotation.y, rotation.z };
        node.scale    = v3{ scale.x,    scale.y,    scale.z    };

        if (m_renderer)
        {
            for (u32 i = 0; i < assimp_node.mNumMeshes; i++)
            {
                const u32 mesh_index = assimp_node.mMeshes[i];
                node.mesh = &m_meshes[mesh_index];
            }
        }

        for (u32 i = 0; i < assimp_node.mNumChildren; i++)
            node.children.emplace_back(visit_node(*assimp_node.mChildren[i]));

        return node;
    }

    void MeshScene::create_meshes(const std::span<aiMesh*> meshes)
    {
        if (!m_renderer)
            return;

        for (const aiMesh* assimp_mesh : meshes)
        {
            std::vector<f32> vertices{};
            std::vector<f32> uvs{};
            std::vector<u32> indices{};

            for (size_t i = 0; i < assimp_mesh->mNumVertices; i++)
            {
                vertices.push_back(assimp_mesh->mVertices[i].x);
                vertices.push_back(assimp_mesh->mVertices[i].y);
                vertices.push_back(assimp_mesh->mVertices[i].z);

                if (assimp_mesh->HasTextureCoords(0))
                {
                    uvs.push_back(assimp_mesh->mTextureCoords[0][i].x);
                    uvs.push_back(assimp_mesh->mTextureCoords[0][i].y);
                }
                else
                {
                    uvs.push_back(0.0f);
                    uvs.push_back(0.0f);
                }
            }

            for (size_t i = 0; i < assimp_mesh->mNumFaces; i++)
            {
                const aiFace& face = assimp_mesh->mFaces[i];
                for (size_t k = 0; k < face.mNumIndices; k++)
                    indices.push_back(face.mIndices[k]);
            }

            auto& [vertex_array, vertex_count] = m_meshes.emplace_back(m_renderer->create_vertex_array_ptr(), 0);

            const auto vertex_buffer = m_renderer->create_buffer_ptr();
            vertex_buffer->update_data(vertices.data(), i32(vertices.size() * sizeof(f32)), gfx::BufferUsage::StaticDraw);

            const auto uv_buffer = m_renderer->create_buffer_ptr();
            uv_buffer->update_data(uvs.data(), i32(uvs.size() * sizeof(f32)), gfx::BufferUsage::StaticDraw);

            const auto index_buffer = m_renderer->create_buffer_ptr();
            index_buffer->update_data(indices.data(), i32(indices.size() * sizeof(u32)), gfx::BufferUsage::StaticDraw);

            vertex_array->attach_vertex_buffer(vertex_buffer, 0, 0, 3 * sizeof(f32));
            vertex_array->setup_attribute_float(0, 0, gfx::AttributeType::F32, false, 3, 0);

            vertex_array->attach_vertex_buffer(uv_buffer, 1, 0, 2 * sizeof(f32));
            vertex_array->setup_attribute_float(1, 1, gfx::AttributeType::F32, false, 2, 0);

            vertex_array->attach_index_buffer(index_buffer);

            vertex_count = i32(indices.size());
        }
    }
}
