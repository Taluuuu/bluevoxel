#include "structure_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_component.h"
#include "input/input_module.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/block_placing_component.h"

namespace bluevoxel
{
    StructureEditorWorkspace::StructureEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_voxel_world_renderer(*this, m_chunk_manager)
        , m_rendering_module(&g_engine->get_module_checked<h2o::RenderingModule>())
    {
        m_chunk_manager.on_chunks_updated.add_listener(m_on_chunks_updated_handle,
            [this](const h2o::ChunksUpdatedEvent&)
            {
                m_extents = calc_extents();
                save_structure();
            }
        );

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

        load_structure(m_selected_structure_id);

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

        if (layer_stack.top_layer() == h2o::Layer::PauseMenu)
            tick_editor_gui();

        if (m_extents == v3i{0})
            set_block_at(v3i{0}, default_block);
    }

    void StructureEditorWorkspace::render()
    {
        auto& renderer = m_rendering_module->renderer();
        renderer.draw_cube(v3{-0.005f}, v3{m_extents} + v3{0.01f}, v4{});
    }

    void StructureEditorWorkspace::tick_editor_gui()
    {
        if (ImGui::Begin("Structure Editor"))
        {
            const auto& structure_mgr = get_structure_mgr();
            const auto structure_names = structure_mgr.structure_names();

            std::vector<const char*> structure_names_c_str{};
            structure_names_c_str.reserve(structure_names.size());
            for (const auto& name : structure_names)
                structure_names_c_str.push_back(name.c_str());

            if (ImGui::Combo("Structure", (i32*)&m_selected_structure_id, structure_names_c_str.data(), i32(structure_names_c_str.size())))
                load_structure(m_selected_structure_id);

            if (ImGui::Button("Create New"))
            {

            }

            ImGui::SameLine();
            if (ImGui::Button("Delete"))
            {

            }

            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                get_voxel_pack().save();
            }

            ImGui::SameLine();
            if (ImGui::Button("Rename"))
            {

            }
        }
        ImGui::End();
    }

    void StructureEditorWorkspace::load_structure(u32 structure_id)
    {
        const auto& structure_mgr = get_structure_mgr();

        const auto structure = structure_mgr.get_structure(structure_id);
        if (!structure)
            return;

        m_chunk_manager.remove_all_chunk_columns([](v2i) { return true; });

        const h2o::VoxelStructureInstance structure_instance{ structure_id, v3i{0} };

        const v3i structure_size = structure->size();
        const v3i structure_size_chunks = structure_size / h2o::voxel_constants::chunk_size;

        h2o::voxel_utils::for_v3i(v3i{0}, structure_size_chunks + v3i{1},
            [&](const v3i& chunk_pos)
            {
                m_chunk_manager.fetch_or_create_chunk_mut(chunk_pos,
                    [&](h2o::Chunk* chunk)
                    {
                        if (chunk)
                            chunk->place_structure(structure_instance);
                    }
                );
            }
        );

        m_chunk_manager.broadcast_events();
    }

    void StructureEditorWorkspace::save_structure()
    {
        auto& structure_mgr = get_structure_mgr();
        if (const auto structure = structure_mgr.get_structure(m_selected_structure_id))
        {
            structure->clear();
            structure->resize(m_extents);

            constexpr v3i max_size_chunks = h2o::voxel_constants::max_structure_size_chunks;
            m_chunk_manager.view<max_size_chunks>(v3i{0},
                [&](const h2o::ChunkView<max_size_chunks>& view)
                {
                    view.for_each_block(
                        [&](const v3i& block_pos, const h2o::Block& block)
                        {
                            structure->set_block(block_pos, block);
                        }
                    );
                }
            );
        }
    }

    v3i StructureEditorWorkspace::calc_extents() const
    {
        v3i extents{};

        constexpr v3i max_size_chunks = h2o::voxel_constants::max_structure_size_chunks;
        m_chunk_manager.view<max_size_chunks>(v3i{0},
            [&](const h2o::ChunkView<max_size_chunks>& view)
            {
                view.for_each_block(
                    [&](const v3i& block_pos, const h2o::Block&)
                    {
                        extents = glm::max(block_pos + v3i{1}, extents);
                    }
                );
            }
        );

        return extents;
    }

    h2o::VoxelPack& StructureEditorWorkspace::get_voxel_pack()
    {
        const auto& voxel_pack = g_engine->get_module_checked<h2o::VoxelModule>().voxel_pack();
        assert(voxel_pack);

        return *voxel_pack;
    }

    h2o::VoxelStructureManager& StructureEditorWorkspace::get_structure_mgr()
    {
        return get_voxel_pack().structure_manager();
    }
}
