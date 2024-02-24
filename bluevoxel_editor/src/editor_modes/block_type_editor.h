#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "voxel/block.h"

#include <memory>
#include <string>

namespace h2o
{
    class RenderingModule;
    class VoxelModule;
}

namespace bluevoxel
{
    class BlockEditorWorkspace;

    class BlockTypeEditor : public h2o::Tickable
    {
    public:

        explicit BlockTypeEditor(BlockEditorWorkspace& workspace);
        ~BlockTypeEditor() override = default;

        void on_voxel_pack_updated();
        void on_selected_block_changed();

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

        // Update combobox names
        void update_block_type_names();
        void update_texture_names();
        void update_preset_names();
        void update_model_names();

    private:

        std::string m_selected_block_name_edit{};

        std::vector<const char*> m_block_type_names_c_str{};
        std::vector<const char*> m_texture_names_c_str{};
        std::vector<const char*> m_block_preset_names_c_str{};
        std::vector<const char*> m_block_model_names_c_str{};

        BlockEditorWorkspace* const m_workspace = nullptr;

        // Module refs
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::VoxelModule* const m_voxel_module = nullptr;

    };
}