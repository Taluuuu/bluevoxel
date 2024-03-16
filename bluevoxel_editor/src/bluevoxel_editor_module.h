#pragma once

#include "block_editor/block_editor_workspace.h"
#include "core/module.h"
#include "core/tickable.h"
#include "voxel/chunk_region.h"
#include "voxel/voxel_bounds.h"
#include "world_gen_editor/world_gen_editor_workspace.h"

#include <memory>
#include <variant>

namespace h2o
{
    class InputModule;
    class VoxelModule;
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

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;
        void render() override;

    private:

        std::variant<nullptr_t, BlockEditorWorkspace, WorldGenEditorWorkspace> m_editor_mode{};

        h2o::InputModule* m_input_module = nullptr;
        h2o::VoxelModule* m_voxel_module = nullptr;

    };
}