#include "structure_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_component.h"
#include "input/input_module.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "voxel_client/block_placing_component.h"

namespace bluevoxel
{
    StructureEditorWorkspace::StructureEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_voxel_world_renderer(*this, m_chunk_manager)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
    {
        // Allocate space for the max structure size
        m_chunk_manager.view_or_create_mut<h2o::voxel_constants::max_structure_size_chunks>(
            v3i{}, [](auto&){});

        m_chunk_manager.on_chunks_updated.add_listener(m_on_chunks_updated_handle,
            [this](const h2o::ChunksUpdatedEvent& event)
            {
                m_extents = calc_extents();
            }
        );

        // Starter block
        StructureEditorWorkspace::set_block_at(v3i{0}, h2o::Block{1});

        m_scene = std::make_shared<h2o::Scene>("editor_scene", nullptr);
        m_scene->add_system<h2o::RenderingSystem>();

        const auto player = m_scene->spawn_actor<h2o::FpsCharacterActor>();

        auto block_placing_comp = player->add_component<h2o::BlockPlacingComponent>();
        block_placing_comp->block_placeable = observer_from_this();

        player->tag_actor(h2o::ActorTag::LocalPlayer);
        player->transform.position = { -2.0f, 0.0f, 0.0f };
        player->transform.rotation = { 0.0f, 0.0f, 0.0f };
        player->transform.scale = { 0.5f, 0.5f, 0.5f };
        player->move_speed = 5.0f;

        set_tick_phases(h2o::TickPhase::Update | h2o::TickPhase::Render);
    }

    void StructureEditorWorkspace::set_block_at(const v3i& block_pos, h2o::Block block)
    {
        m_chunk_manager.set_block_at(block_pos, block);
        m_chunk_manager.broadcast_events();
    }

    void StructureEditorWorkspace::update(f32 delta_time)
    {
        // Mouse capture toggle
        auto& layer_stack = g_engine->layer_stack();
        const auto& input = g_engine->get_module_checked<h2o::InputModule>();
        if (input.key_state(h2o::Key::Escape).pressed_this_frame)
        {
            if (layer_stack.top_layer() == h2o::Layer::PauseMenu)
            {
                layer_stack.pop_layer(h2o::Layer::PauseMenu);
            }
            else
            {
                layer_stack.push_layer(h2o::Layer::PauseMenu,
                    h2o::LayerData{ false, true });
            }
        }
    }

    void StructureEditorWorkspace::render()
    {
        auto& renderer = m_rendering_module->renderer();
        renderer.draw_cube(v3{-0.005f}, v3{m_extents} + v3{0.01f}, v4{});
    }

    v3i StructureEditorWorkspace::calc_extents() const
    {
        v3i extents{};

        // TODO: Add for_each_block to ChunkManager ?
        for (i32 i = 0; i < h2o::voxel_constants::max_structure_size_chunks.x; i++)
        for (i32 j = 0; j < h2o::voxel_constants::max_structure_size_chunks.y; j++)
        for (i32 k = 0; k < h2o::voxel_constants::max_structure_size_chunks.z; k++)
        {
            const v3i chunk_pos{ i, j, k };
            const v3i chunk_corner = chunk_pos * h2o::voxel_constants::chunk_size;

            m_chunk_manager.fetch_chunk(chunk_pos,
                [&](const h2o::Chunk* chunk)
                {
                    if (!chunk)
                        return;

                    for (i32 ii = 0; ii < h2o::voxel_constants::chunk_size; ii++)
                    for (i32 jj = 0; jj < h2o::voxel_constants::chunk_size; jj++)
                    for (i32 kk = 0; kk < h2o::voxel_constants::chunk_size; kk++)
                    {
                        const v3i local_block_pos{ ii, jj, kk };
                        if (chunk->get_block_at(local_block_pos) != h2o::Block::Air)
                        {
                            const v3i block_pos = local_block_pos + chunk_corner;
                            extents = glm::max(block_pos + v3i{1}, extents);
                        }
                    }
                }
            );
        }

        // for (i32 i = 0; i < h2o::voxel_constants::max_structure_size_blocks.x; i++)
        // for (i32 j = 0; j < h2o::voxel_constants::max_structure_size_blocks.y; j++)
        // for (i32 k = 0; k < h2o::voxel_constants::max_structure_size_blocks.z; k++)
        // {
        //     const v3i block_pos{ i, j, k };
        //     if (const auto block = m_chunk_manager.get_block_at(block_pos); block && block != h2o::Block::Air)
        //         extents = glm::max(block_pos + v3i{1}, extents);
        // }

        return extents;
    }
}
