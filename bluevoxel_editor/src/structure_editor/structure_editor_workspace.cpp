#include "structure_editor_workspace.h"

#include "core/engine.h"
#include "game_framework/actors/fps_character_actor.h"
#include "input/input_component.h"
#include "input/input_module.h"
#include "rendering/renderer.h"
#include "rendering/rendering_module.h"
#include "scene/scene.h"
#include "scene_rendering/rendering_scene_system.h"
#include "ui/imgui.h"
#include "voxel/voxel_pack.h"
#include "voxel_client/block_placing_component.h"

namespace bluevoxel
{
    static constexpr const char* rename_structure_modal_name = "Name Selected Structure";
    static constexpr const char* delete_structure_modal_name = "Delete Selected Structure?";

    StructureEditorWorkspace::StructureEditorWorkspace(h2o::Tickable* owner)
        : h2o::Tickable(owner)
        , m_voxel_world_renderer(*this, m_chunk_manager, h2o::ChunkRenderMode::DrawAllChunks)
        , m_selection_mgr(owner)
        , m_structure_gizmo(owner, m_selection_mgr)
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

        player->fly = true;
        player->fly_speed = 5.0f;

        m_structure_gizmo.increment_size = 1.0f;
        m_structure_gizmo.reset_position_on_release = true;

        load_selected_structure();

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

        if (layer_stack.top_layer() >= h2o::Layer::PauseMenu)
        {
            tick_editor_gui();
            m_structure_gizmo.enable();

            m_structure_gizmo.bounds = Gizmo::Bounds{
                -h2o::voxel_constants::max_structure_size_blocks,
                h2o::voxel_constants::max_structure_size_blocks };

            if (const auto structure = get_selected_structure())
            {
                if (m_structure_gizmo.movement_delta() != v3{})
                {
                    if (structure->move(m_structure_gizmo.movement_delta()))
                        load_selected_structure();
                }
            }
        }
        else
        {
            m_structure_gizmo.disable();
        }

        if (m_extents == v3i{0})
            set_block_at(v3i{0}, default_block);
    }

    void StructureEditorWorkspace::render()
    {
        if (m_extents.x && m_extents.y && m_extents.z)
        {
            auto& renderer = m_rendering_module->renderer();
            renderer.draw_cube(v3{-0.005f}, v3{m_extents} + v3{0.01f}, v4{});
        }
    }

    void StructureEditorWorkspace::tick_editor_gui()
    {
        auto& structure_mgr = get_structure_mgr();

        if (ImGui::Begin("Structure Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto structure_names = structure_mgr.structure_names();

            std::vector<const char*> structure_names_c_str{};
            structure_names_c_str.reserve(structure_names.size());
            for (const auto& name : structure_names)
                structure_names_c_str.push_back(name.c_str());

            if (ImGui::Combo("Structure", (i32*)&m_selected_structure_id, structure_names_c_str.data(), i32(structure_names_c_str.size())))
                load_selected_structure();

            ImGui::Text("ID: %u", m_selected_structure_id);
            ImGui::Text("Size: (%i, %i, %i)", m_extents.x, m_extents.y, m_extents.z);

            if (ImGui::Button("Create New"))
            {
                m_selected_structure_id = structure_mgr.add_structure(h2o::VoxelStructure{ "no_name" });
                load_selected_structure();
                open_rename_structure_popup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete"))
            {
                open_delete_structure_popup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                get_voxel_pack().save();
            }

            ImGui::SameLine();
            if (ImGui::Button("Rename"))
            {
                open_rename_structure_popup();
            }
        }

        // Structure naming pop-up
        if (ImGui::BeginPopupModal(rename_structure_modal_name))
        {
            ImGui::InputText("New Name", &m_selected_structure_name_edit);

            if (ImGui::Button("Confirm"))
            {
                structure_mgr.rename_structure(m_selected_structure_id, m_selected_structure_name_edit);
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        // Structure deleting pop-up
        if (ImGui::BeginPopupModal(delete_structure_modal_name))
        {
            if (ImGui::Button("Confirm"))
            {
                m_selected_structure_id = structure_mgr.delete_structure(m_selected_structure_id).value_or(0);
                load_selected_structure();

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void StructureEditorWorkspace::load_selected_structure()
    {
        m_chunk_manager.remove_all_chunk_columns([](v2i) { return true; });

        if (const auto structure = get_selected_structure())
        {
            const h2o::VoxelStructureInstance structure_instance{ m_selected_structure_id, v3i{0} };

            const v3i structure_size = structure->size();
            const v3i structure_size_chunks = structure_size / h2o::voxel_constants::chunk_size;

            h2o::voxel_utils::for_v3i(v3i{0}, structure_size_chunks + v3i{1},
                [&](const v3i& chunk_pos)
                {
                    m_chunk_manager.fetch_or_create_chunk_mut(chunk_pos,
                        [&](h2o::Chunk* chunk)
                        {
                            if (chunk)
                            {
                                chunk->place_structure(structure_instance);
                                chunk->mark_generated();
                            }
                        }
                    );
                }
            );
        }

        m_extents = calc_extents();
        m_chunk_manager.broadcast_events();
    }

    void StructureEditorWorkspace::save_structure()
    {
        if (const auto structure = get_selected_structure())
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

    void StructureEditorWorkspace::open_rename_structure_popup()
    {
        if (const auto structure = get_selected_structure())
        {
            m_selected_structure_name_edit = structure->name();
            ImGui::OpenPopup(rename_structure_modal_name);
        }
    }

    void StructureEditorWorkspace::open_delete_structure_popup()
    {
        ImGui::OpenPopup(delete_structure_modal_name);
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

    h2o::VoxelStructure* StructureEditorWorkspace::get_selected_structure() const
    {
        return get_structure_mgr().get_structure(m_selected_structure_id);
    }
}
