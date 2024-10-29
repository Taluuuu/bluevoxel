#pragma once

#include "core/handle_types.h"
#include "graph/graph_ui.h"
#include "networking/net_peer_local.h"

#include <memory>


namespace h2o
{
    class ChunkClient;
    class ChunkServer;
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
        void regenerate();

    private:

        h2o::WeakHandle<h2o::ChunkClient> m_chunk_client = nullptr;
        h2o::WeakHandle<h2o::ChunkServer> m_chunk_server = nullptr;

        h2o::GraphUIContext m_graph_ui_context{};

        h2o::NetPeer_Local m_local_net_peer{};

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

    };
}