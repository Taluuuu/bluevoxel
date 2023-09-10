#pragma once

#include "core/module.h"

#include <set>

namespace h2o
{
    class Client;
    class Server;

    class NetworkingModule : public IModule
    {
    public:

        ~NetworkingModule() override = default;

        void register_server(Server& server);
        void unregister_server(Server& server);

        void register_client(Client& client);
        void unregister_client(Client& client);

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_networking_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        std::set<Server*> m_active_servers{};
        std::set<Client*> m_connected_clients{};

    };
}