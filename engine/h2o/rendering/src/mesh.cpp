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
    bool Mesh::load(const std::string& path)
    {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate      |
            aiProcess_JoinIdenticalVertices   |
            aiProcess_FlipUVs                 |
            aiProcess_GlobalScale             |
            aiProcess_PreTransformVertices);

        if (!scene)
        {
            log::error("Failed to import mesh at '{}': {}.", path, importer.GetErrorString());
            return false;
        }

        const auto& rendering_module = g_engine->get_module_checked<RenderingModule>();
        auto& renderer = rendering_module.renderer();

        std::vector<f32> vertices{};
        std::vector<f32> uvs{};
        std::vector<u32> indices{};
        u32 indices_offset = 0;
        for (size_t i = 0; i < scene->mNumMeshes; i++)
        {
            const auto mesh = scene->mMeshes[i];

            for (size_t j = 0; j < mesh->mNumVertices; j++)
            {
                const auto& vertex = mesh->mVertices[j];
                vertices.push_back(vertex.x);
                vertices.push_back(vertex.y);
                vertices.push_back(vertex.z);

                const auto& tex_coord = mesh->mTextureCoords[0][j];
                uvs.push_back(tex_coord.x);
                uvs.push_back(tex_coord.y);
            }

            for (size_t j = 0; j < mesh->mNumFaces; j++)
            {
                const auto& face = mesh->mFaces[j];
                for (size_t k = 0; k < face.mNumIndices; k++)
                    indices.push_back(face.mIndices[k] + indices_offset);
            }

            indices_offset += (vertices.size() / 3);
        }

        auto vertex_buffer = renderer.create_buffer_OLD();
        vertex_buffer->update_data(vertices.data(), i32(vertices.size() * sizeof(f32)));

        auto uv_buffer = renderer.create_buffer_OLD();
        uv_buffer->update_data(uvs.data(), i32(uvs.size() * sizeof(f32)));

        auto index_buffer = renderer.create_buffer_OLD();
        index_buffer->update_data(indices.data(), i32(indices.size() * sizeof(u32)));

        m_vertex_array = renderer.create_vertex_array_OLD();
        m_vertex_array->attach_vertex_buffer(vertex_buffer, 0, 0, 3 * sizeof(f32));
        m_vertex_array->setup_attribute(0, 0, AttributeType::F32, 3, 0);

        m_vertex_array->attach_vertex_buffer(uv_buffer, 1, 0, 2 * sizeof(f32));
        m_vertex_array->setup_attribute(1, 1, AttributeType::F32, 2, 0);

        m_vertex_array->attach_index_buffer(index_buffer);

        m_vertex_count = i32(indices.size());

        importer.FreeScene();

        return true;
    }
}