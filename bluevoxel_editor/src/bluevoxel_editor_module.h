#pragma once

#include "core/module.h"
#include "core/tickable.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_bounds.h"

#include <memory>

namespace h2o
{
    class Chunk;
    class ChunkMeshPool;
    class Scene;

    class InputModule;
    class RenderingModule;
    class UIModule;
    class VoxelRenderingModule;
}

namespace bluevoxel
{
    class BlueVoxelEditorModule
        : public h2o::Tickable
        , public h2o::IModule
    {
    public:

        BlueVoxelEditorModule();
        ~BlueVoxelEditorModule() override = default;

        // h2o::IModule interface
        bool init(h2o::Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "bluevoxel_editor"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    public:

        v3 light_dir { 0.18f, -1.0f, 0.492f };
        v3 light_color { 1.0f, 1.0f, 1.0f };
        f32 ambient_strength = 0.714f;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;
        void render() override;

    private:

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        std::shared_ptr<h2o::Chunk> m_chunk = nullptr;

        h2o::ChunkRegion m_chunk_region;
        h2o::VoxelBounds m_voxel_bounds;
        std::shared_ptr<h2o::ChunkMeshPool> m_chunk_mesh_pool = nullptr;

        h2o::InputModule* m_input_module = nullptr;
        h2o::RenderingModule* m_rendering_module = nullptr;
        h2o::UIModule* m_ui_module = nullptr;
        h2o::VoxelRenderingModule* m_voxel_rendering_module = nullptr;

    };
}