#pragma once

#include "scene/actor.h"
#include "core/core_interfaces.h"
#include "rendering/pipeline.h"
#include "rendering/vertex_array.h"

namespace h2o
{
    class TriangleActor
        : public Actor
        , public ITickable
    {
    public:

        explicit TriangleActor(const ActorInitializer& actor_initializer);
        ~TriangleActor() override = default;

        void tick(TickPhase phase, f64 delta_time) override;

    private:

        std::shared_ptr<h2o::gfx::IPipeline>    m_pipeline     = nullptr;
        std::shared_ptr<h2o::gfx::IVertexArray> m_vertex_array = nullptr;

    };
}