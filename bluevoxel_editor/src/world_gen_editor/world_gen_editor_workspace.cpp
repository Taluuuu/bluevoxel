#include "world_gen_editor_workspace.h"

namespace bluevoxel
{
    WorldGenEditorWorkspace::WorldGenEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_server(owner)
        , m_client(owner)
    {
        m_server.start(1234, true);

        set_tick_phases(h2o::TickPhase::Update);
    }

    void WorldGenEditorWorkspace::update(f32 delta_time)
    {
        m_time_since_start += delta_time;
        if (m_time_since_start > time_before_connecting && !test)
        {
            m_client.connect("127.0.0.1", 1234);
            test = true;
        }
    }
}