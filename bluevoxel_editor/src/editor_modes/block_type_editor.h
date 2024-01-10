#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_bounds.h"

#include <memory>
#include <string>

namespace h2o
{
    class Chunk;
    class ChunkMeshPool;
    class Scene;

    class RenderingModule;
    class UIModule;
    class VoxelModule;
    class VoxelRenderingModule;
}

namespace bluevoxel
{
    class BlockTypeEditor : public h2o::Tickable
    {
    public:

        explicit BlockTypeEditor(h2o::Tickable* owner);
        ~BlockTypeEditor() override = default;

    public:

        v3 light_dir { 0.18f, -1.0f, 0.492f };
        v3 light_color { 1.0f, 1.0f, 1.0f };
        f32 ambient_strength = 0.714f;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;
        void render() override;

        void on_changed_block_type_selection(h2o::BlockID block_id);
        void on_voxel_pack_changed();
        void on_voxel_pack_updated();

        void update_block_type_names();
        void update_texture_names();
        void update_preset_names();
        void update_model_names();

        void reload_voxel_pack();

    private:

        u32 m_selected_block_id = 0;
        std::optional<std::string> m_selected_block_name_edit{};

        std::vector<const char*> m_block_type_names_c_str{};
        std::vector<const char*> m_texture_names_c_str{};
        std::vector<const char*> m_block_preset_names_c_str{};
        std::vector<const char*> m_block_model_names_c_str{};

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        // Chunk rendering
        std::shared_ptr<h2o::Chunk> m_chunk = nullptr;
        h2o::ChunkRegion m_chunk_region;
        h2o::VoxelBounds m_voxel_bounds;
        std::shared_ptr<h2o::ChunkMeshPool> m_chunk_mesh_pool = nullptr;

        // Module refs
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::UIModule* const m_ui_module = nullptr;
        h2o::VoxelModule* const m_voxel_module = nullptr;
        h2o::VoxelRenderingModule* const m_voxel_rendering_module = nullptr;

        // Event handles
        h2o::EventHandle m_on_voxel_pack_changed_event_handle{};
        h2o::EventHandle m_on_voxel_pack_updated_event_handle{};

    };
}