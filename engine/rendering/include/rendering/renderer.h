#pragma once

namespace engine
{
    struct GameInfo;
    class IWindow;

    class IRenderer
    {
    public:

        virtual ~IRenderer() {}

        virtual void draw_frame() const = 0;

    };
}