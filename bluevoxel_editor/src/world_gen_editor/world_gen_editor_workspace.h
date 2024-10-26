#pragma once

#include "networking/net_peer_local.h"

#include <memory>
#include <vector>

namespace h2o
{
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

        struct PointGraph
        {
            std::vector<v2> points{};
        };

        PointGraph m_point_graph{};

        h2o::NetPeer_Local m_local_net_peer{};

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

    };
}