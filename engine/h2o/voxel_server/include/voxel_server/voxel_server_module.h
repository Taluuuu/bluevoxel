#pragma once

#include "core/module.h"
#include "chunk_server.h"

namespace h2o
{
    class VoxelServerModule : public IModule
    {
    public:

        ~VoxelServerModule() override = default;

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override;
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

//    private:
//
//        ChunkServer m_chunk_server{};

    };
}