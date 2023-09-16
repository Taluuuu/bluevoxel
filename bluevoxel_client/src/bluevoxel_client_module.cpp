#include "bluevoxel_client_module.h"

#include "core/engine.h"
#include "networking/networking_module.h"
#include "rendering/rendering_module.h"
#include "voxel_client/voxel_client_module.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace bluevoxel
{
    void BlueVoxelClientModule::update(f32 delta_time)
    {
        switch (m_client->connection_state())
        {
        case h2o::ConnectionState::Disconnected:
        {
            ImGui::Begin("Connect to Server");

                ImGui::InputText("Server IP", &m_server_ip);
                if (ImGui::InputInt("Server Port", &m_server_port, 0))
                    m_server_port = glm::clamp(m_server_port, 0, 65'535);

                if (ImGui::Button("Connect to server"))
                    m_client->connect(m_server_ip, m_server_port);

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

                if (ImGui::Button("Send packet"))
                {
                    h2o::TestMessage msg { "texte :))", 43843 };
                    m_client->send_message(msg);
                }

                if (ImGui::Button("Disconnect from Server"))
                    m_client->disconnect();

            ImGui::End();
            break;
        }
        }
    }

    bool BlueVoxelClientModule::init(h2o::Engine& engine)
    {
        m_client.emplace();

        set_tick_phases(h2o::Update);

        if (const auto networking_module = engine.get_module<h2o::NetworkingModule>())
        {
            networking_module->on_client_created.add_listener(m_on_client_created,
                [](const h2o::ClientCreatedEvent& event)
                {
//                    event.client.bind_on_received_message<h2o::TestMessage>(m_on);
                });
        }

        return true;
    }

    void BlueVoxelClientModule::cleanup()
    {
        m_client->disconnect();
        m_client.reset();
    }

    std::vector<std::type_index> BlueVoxelClientModule::dependencies() const
    {
        return {
            typeid(h2o::VoxelClientModule),
            typeid(h2o::RenderingModule)
        };
    }
}