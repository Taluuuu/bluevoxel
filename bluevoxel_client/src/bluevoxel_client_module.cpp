#include "bluevoxel_client_module.h"

#include "rendering/rendering_module.h"
#include "voxel_client/voxel_client_module.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace bluevoxel
{
    void BlueVoxelClientModule::update(f32 delta_time)
    {
        switch (m_client.connection_state())
        {
        case h2o::ConnectionState::Disconnected:
        {
            ImGui::Begin("Connect to Server");

                ImGui::InputText("Server IP", &m_server_ip);
                if (ImGui::InputInt("Server Port", &m_server_port, 0))
                    m_server_port = glm::clamp(m_server_port, 0, 65'535);

                if (ImGui::Button("Connect to server"))
                    m_client.connect(m_server_ip, m_server_port);

            ImGui::End();
            break;
        }

        case h2o::ConnectionState::Connecting:
        {
            ImGui::Begin("Connection");

                ImGui::Text("Connecting...");

            ImGui::End();
        }

        case h2o::ConnectionState::Connected:
        {
            ImGui::Begin("Connection");

                if (ImGui::Button("Disconnect from Server"))
                    m_client.disconnect();

            ImGui::End();
            break;
        }
        }
    }

    bool BlueVoxelClientModule::init(h2o::Engine& engine)
    {
        set_tick_phases(h2o::Update);

        return true;
    }

    void BlueVoxelClientModule::cleanup()
    {
        m_client.disconnect();
    }

    std::vector<std::type_index> BlueVoxelClientModule::dependencies() const
    {
        return {
            typeid(h2o::VoxelClientModule),
            typeid(h2o::RenderingModule)
        };
    }
}