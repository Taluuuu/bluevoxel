#pragma once

#include "networking/net_peer_local.h"

#include <memory>

namespace h2o
{
    class InputModule;
    class Scene;
}

namespace bluevoxel
{
    class WorldGenEditorWorkspace : public h2o::Tickable
    {
    public:

        explicit WorldGenEditorWorkspace(h2o::Tickable* owner);
        ~WorldGenEditorWorkspace() override = default;

    protected:

        // h2o::Tickable interface
        void update(f32 delta_time) override;

    private:

        h2o::NetPeer_Local m_local_net_peer{};

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        h2o::InputModule* const m_input_module = nullptr;

    };
}