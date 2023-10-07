#include "../include/voxel_client/block_placing_component.h"

#include "input/input_component.h"
#include "rendering/camera.h"
#include "scene/actor.h"
#include "scene/scene.h"
#include "scene_rendering/camera_component.h"
#include "voxel/chunk.h"
#include "voxel/chunk_system.h"
#include "voxel/voxel_ray.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    BlockPlacingComponent::BlockPlacingComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {
        set_tick_phases(TickPhase_Update);

        m_camera = owner()->get_component<CameraComponent>();
        m_input = owner()->get_component<InputComponent>();
    }

    void BlockPlacingComponent::update(f32 delta_time)
    {
//        assert(m_camera);
//        assert(m_input);
//
//        const auto chunk_rendering_system = m_scene->get_system<ChunkRenderingSystem>();
//        if (!chunk_rendering_system)
//            return;
//
//        const auto chunk_system = m_scene->get_system<ChunkSystem>();
//        if (!chunk_system)
//            return;
//
//        const auto rendering_region = chunk_rendering_system->rendering_region();
//        if (!rendering_region)
//            return;
//
//        const v3 front = m_camera->camera().calc_front(); // TODO: Cache front vector...
//        const v3 origin = owner()->transform.position;
//        const v3 end = origin + front * 15.0f; // Magic number :))
//
//        if (const VoxelRay ray { origin, end, *rendering_region })
//        {
//            if (m_input->mouse_button_state(MouseButton::Left).pressed_this_frame)
//            {
//                const auto& hit_voxel = ray.hit().hit_voxel;
//                hit_voxel.chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(hit_voxel.pos), Block::Air);
//
//                chunk_system->on_chunk_updated.broadcast({ hit_voxel.chunk });
//            }
//
//            if (m_input->mouse_button_state(MouseButton::Right).pressed_this_frame)
//            {
//                const auto& hit_voxel = ray.hit().before_hit_voxel;
//                if (hit_voxel.chunk)
//                {
//                    hit_voxel.chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(hit_voxel.pos), Block { 2 });
//                    chunk_system->on_chunk_updated.broadcast({hit_voxel.chunk});
//                }
//            }
//        }
    }
}