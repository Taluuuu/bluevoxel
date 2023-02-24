#pragma once

#include "rendering/renderer.h"

namespace engine
{
    class Renderer_OpenGL : public IRenderer
    {

        // IRenderer interface
    public:
        PipelineCreateData         create_pipeline()                                         override;
        std::shared_ptr<IPipeline> compile_pipeline(const PipelineCreateData& create_data)   override;
        void                       bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) override;
    protected:
        bool init(const GameInfo& game_info, IWindow& window)                                override;
        void draw_frame()                                                                    override;
        // End IRenderer interface

    private:

        std::shared_ptr<IPipeline> m_bound_pipeline = nullptr;

    };
}