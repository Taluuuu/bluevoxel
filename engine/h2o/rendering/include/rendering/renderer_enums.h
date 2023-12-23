#pragma once

namespace h2o::gfx
{
    enum class BufferUsage
    {
        StaticDraw,
        StreamDraw
    };

    enum class ShaderStage
    {
        Vertex,
        Fragment,
        Geometry,
    };

    namespace PipelineFeature
    {
        enum Type : u32
        {
            None = 0,
            Blend = 1 << 0,
            CullFace = 1 << 1,
            DepthTest = 1 << 2,
            ScissorTest = 1 << 3,
        };

        inline PipelineFeature::Type operator|(PipelineFeature::Type lhs, PipelineFeature::Type rhs)
        {
            return static_cast<PipelineFeature::Type>(static_cast<u32>(lhs) | static_cast<u32>(rhs));
        }
    }

    enum class BlendEquation
    {
        Add
    };

    enum class BlendFactor
    {
        SrcAlpha,
        OneMinusSrcAlpha
    };

    enum class AttributeType
    {
        F32,
        U16,
        U32,
    };
}