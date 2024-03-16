#pragma once

#include "networking/server.h"
#include "networking/client.h"

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

        static constexpr f32 time_before_connecting = 1.0f;
        f32 m_time_since_start = 0.0f;

        h2o::Server m_server;
        h2o::Client m_client;

        bool test = false;

    };
}