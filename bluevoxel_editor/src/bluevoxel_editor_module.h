#pragma once

#include "core/module.h"
#include "core/tickable.h"

namespace h2o
{
    class UIModule;
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

        h2o::UIModule* m_ui_module = nullptr;

    };
}