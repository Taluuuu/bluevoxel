#include "rendering/renderer.h"

namespace h2o::gfx
{
    void IRenderer::draw_cube(const v3& origin, const v3& extents, const v4& color)
    {
        const std::array<v3, 8> points
        {
            v3{ origin + v3{ 0.0f, 0.0f, 0.0f } * extents },
            v3{ origin + v3{ 0.0f, 0.0f, 1.0f } * extents },
            v3{ origin + v3{ 0.0f, 1.0f, 0.0f } * extents },
            v3{ origin + v3{ 0.0f, 1.0f, 1.0f } * extents },
            v3{ origin + v3{ 1.0f, 0.0f, 0.0f } * extents },
            v3{ origin + v3{ 1.0f, 0.0f, 1.0f } * extents },
            v3{ origin + v3{ 1.0f, 1.0f, 0.0f } * extents },
            v3{ origin + v3{ 1.0f, 1.0f, 1.0f } * extents },
        };

        draw_line(points[0], points[1], color);
        draw_line(points[0], points[2], color);
        draw_line(points[0], points[4], color);
        draw_line(points[1], points[3], color);
        draw_line(points[1], points[5], color);
        draw_line(points[2], points[3], color);
        draw_line(points[2], points[6], color);
        draw_line(points[4], points[5], color);
        draw_line(points[4], points[6], color);
        draw_line(points[3], points[7], color);
        draw_line(points[5], points[7], color);
        draw_line(points[6], points[7], color);
    }
}
