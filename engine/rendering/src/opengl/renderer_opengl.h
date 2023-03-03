#pragma once

#include "core/events.h"
#include "rendering/renderer.h"

namespace h2o::gfx
{
    class Pipeline_OpenGL;

    class Renderer_OpenGL : public IRenderer
    {
        // IRenderer interface
    public:
        PipelineCreateData            create_pipeline()                                         override;
        std::shared_ptr<IPipeline>    compile_pipeline(const PipelineCreateData& create_data)   override;
        void                          bind_pipeline(const std::shared_ptr<IPipeline>& pipeline) override;
        std::shared_ptr<IBuffer>      create_buffer()                                           override;
        std::shared_ptr<IVertexArray> create_vertex_array()                                     override;
        void                          draw(const IVertexArray& vertex_array)                    override;
    protected:
        bool init(IWindow& window, const GameInfo& game_info) override;
        void clear() override;

    private:

        std::shared_ptr<Pipeline_OpenGL> m_bound_pipeline = nullptr;

        EventHandle m_window_resize_event_handle;

    };
}