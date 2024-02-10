#include "voxel_client/block_placing_component.h"

#include "core/engine.h"
#include "input/input_component.h"
#include "rendering/camera.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/camera_component.h"
#include "voxel/voxel_ray.h"
#include "voxel_client/chunk_client.h"

namespace h2o
{
    BlockPlacingComponent::BlockPlacingComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
        , m_rendering_module(&g_engine->get_module_checked<RenderingModule>())
    {
        set_tick_phases(TickPhase::Update);

        m_camera = owner()->get_component<CameraComponent>();
        m_input = owner()->get_component<InputComponent>();
    }

    void BlockPlacingComponent::update(f32 delta_time)
    {
        assert(m_camera);
        assert(m_input);

        const auto chunk_client = m_scene->get_system<ChunkClient>();
        if (!chunk_client)
            return;

        auto& chunk_mgr = chunk_client->chunk_mgr();

        const v3 front = m_camera->camera().front();
        const v3 origin = owner()->transform.position;
        const v3 end = origin + front * 15.0f;

        if (const VoxelRay ray { origin, end, chunk_client->chunk_mgr() })
        {
            const auto& [hit_voxel, before_hit_voxel] = ray.hit();
            auto& renderer = m_rendering_module->renderer();
            const v4 line_color{ 0.0f, 0.0f, 0.0f, 1.0f };
            const v3i pos = hit_voxel.pos;

            std::array<v3, 8> points
            {
                v3{ pos + v3i{ 0, 0, 0 } } - front * 0.01f,
                v3{ pos + v3i{ 0, 0, 1 } } - front * 0.01f,
                v3{ pos + v3i{ 0, 1, 0 } } - front * 0.01f,
                v3{ pos + v3i{ 0, 1, 1 } } - front * 0.01f,
                v3{ pos + v3i{ 1, 0, 0 } } - front * 0.01f,
                v3{ pos + v3i{ 1, 0, 1 } } - front * 0.01f,
                v3{ pos + v3i{ 1, 1, 0 } } - front * 0.01f,
                v3{ pos + v3i{ 1, 1, 1 } } - front * 0.01f,
            };

            renderer.draw_line(points[0], points[1], line_color);
            renderer.draw_line(points[0], points[2], line_color);
            renderer.draw_line(points[0], points[4], line_color);
            renderer.draw_line(points[1], points[3], line_color);
            renderer.draw_line(points[1], points[5], line_color);
            renderer.draw_line(points[2], points[3], line_color);
            renderer.draw_line(points[2], points[6], line_color);
            renderer.draw_line(points[4], points[5], line_color);
            renderer.draw_line(points[4], points[6], line_color);
            renderer.draw_line(points[3], points[7], line_color);
            renderer.draw_line(points[5], points[7], line_color);
            renderer.draw_line(points[6], points[7], line_color);

            for (size_t i = 0; i < 4; i++)
            {
                renderer.draw_line(points[0], points[1], line_color);
                renderer.draw_line(points[0], points[2], line_color);
                renderer.draw_line(points[0], points[4], line_color);
            }

            if (m_input->mouse_button_state(MouseButton::Left).pressed_this_frame)
            {
                chunk_mgr.set_block_at(hit_voxel.pos, Block::Air, true);
            }

            if (m_input->mouse_button_state(MouseButton::Right).pressed_this_frame)
            {
                chunk_mgr.set_block_at(before_hit_voxel.pos, Block { 2 }, true);
            }
        }
    }
}