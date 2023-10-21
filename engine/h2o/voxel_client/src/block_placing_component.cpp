#include "voxel_client/block_placing_component.h"

#include "input/input_component.h"
#include "rendering/camera.h"
#include "scene/actor.h"
#include "scene/scene.h"
#include "scene_rendering/camera_component.h"
#include "voxel/chunk.h"
#include "voxel/voxel_ray.h"
#include "voxel/voxel_utils.h"
#include "voxel_client/chunk_client.h"

namespace h2o
{
    BlockPlacingComponent::BlockPlacingComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
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

        const v3 front = m_camera->camera().calc_front(); // TODO: Cache front vector...
        const v3 origin = owner()->transform.position;
        const v3 end = origin + front * 15.0f;

        if (const VoxelRay ray { origin, end, chunk_client->chunk_mgr() })
        {
            const auto& [hit_voxel, before_hit_voxel] = ray.hit();
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