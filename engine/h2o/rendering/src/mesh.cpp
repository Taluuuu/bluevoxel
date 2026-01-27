#include "rendering/mesh.h"

#include "core/engine.h"
#include "rendering/buffer.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "rendering/vertex_array.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace h2o::gfx
{
    namespace
    {
        void visit_node(
            const aiNode* node,
            const aiMatrix4x4& parent_transform,
            aiMesh** meshes,
            std::vector<f32>& out_vertices,
            std::vector<f32>& out_uvs,
            std::vector<u32>& out_indices)
        {
            assert(node != nullptr);

            const aiMatrix4x4 global_transform = parent_transform * node->mTransformation;

            for (u32 i = 0; i < node->mNumMeshes; i++)
            {
                const u32 mesh_index = node->mMeshes[i];
                const auto mesh = meshes[mesh_index];

                const u32 current_mesh_vertex_offset = static_cast<u32>(out_vertices.size() / 3);

                for (size_t j = 0; j < mesh->mNumVertices; j++)
                {
                    const aiVector3D vertex = global_transform * mesh->mVertices[j];
                    out_vertices.push_back(vertex.x);
                    out_vertices.push_back(vertex.y);
                    out_vertices.push_back(vertex.z);

                    if (mesh->HasTextureCoords(0))
                    {
                        out_uvs.push_back(mesh->mTextureCoords[0][j].x);
                        out_uvs.push_back(mesh->mTextureCoords[0][j].y);
                    }
                    else
                    {
                        out_uvs.push_back(0.0f);
                        out_uvs.push_back(0.0f);
                    }
                }

                for (size_t j = 0; j < mesh->mNumFaces; j++)
                {
                    const aiFace& face = mesh->mFaces[j];
                    for (size_t k = 0; k < face.mNumIndices; k++)
                        out_indices.push_back(face.mIndices[k] + current_mesh_vertex_offset);
                }
            }

            for (u32 i = 0; i < node->mNumChildren; i++)
                visit_node(node->mChildren[i], global_transform, meshes, out_vertices, out_uvs, out_indices);
        }
    }

    bool Mesh::load(const fs::path& path)
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

        const auto& rendering_module = g_engine->get_module_checked<RenderingModule>();
        auto& renderer = rendering_module.renderer();

        std::vector<f32> vertices{};
        std::vector<f32> uvs{};
        std::vector<u32> indices{};
        u32 indices_offset = 0;

        visit_node(scene->mRootNode, aiMatrix4x4{}, scene->mMeshes, vertices, uvs, indices);

        auto vertex_buffer = renderer.create_buffer_ptr();
        vertex_buffer->update_data(vertices.data(), i32(vertices.size() * sizeof(f32)), gfx::BufferUsage::StaticDraw);

        auto uv_buffer = renderer.create_buffer_ptr();
        uv_buffer->update_data(uvs.data(), i32(uvs.size() * sizeof(f32)), gfx::BufferUsage::StaticDraw);

        auto index_buffer = renderer.create_buffer_ptr();
        index_buffer->update_data(indices.data(), i32(indices.size() * sizeof(u32)), gfx::BufferUsage::StaticDraw);

        m_vertex_array = renderer.create_vertex_array_ptr();
        m_vertex_array->attach_vertex_buffer(vertex_buffer, 0, 0, 3 * sizeof(f32));
        m_vertex_array->setup_attribute_float(0, 0, AttributeType::F32, false, 3, 0);

        m_vertex_array->attach_vertex_buffer(uv_buffer, 1, 0, 2 * sizeof(f32));
        m_vertex_array->setup_attribute_float(1, 1, AttributeType::F32, false, 2, 0);

        m_vertex_array->attach_index_buffer(index_buffer);

        m_vertex_count = i32(indices.size());

        importer.FreeScene();

        return true;
    }
}