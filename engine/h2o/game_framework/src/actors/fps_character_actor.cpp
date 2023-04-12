#include "game_framework/actors/fps_character_actor.h"

#include "scene_rendering/camera_component.h"
#include "input/input_component.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    FpsCharacterActor::FpsCharacterActor(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        add_component<CameraComponent>();
        m_input = add_component<InputComponent>();

        set_tick_phases(Update);
    }

    void FpsCharacterActor::update(f32 delta_time)
    {
        if (!m_input) return;

        const v3 cam_input {
            m_input->get_axis("cam_x"),
            m_input->get_axis("cam_y"), 0.0f };

        transform.rotation += cam_input;
        transform.rotation.x = glm::clamp(transform.rotation.x, -89.0f, 89.0f);
        
        v2 move_input {
            m_input->get_axis("move_y"),
            m_input->get_axis("move_x") };

        if (glm::length2(move_input) > 0.1f)
            move_input = glm::normalize(move_input);

        const f32 rot_y = glm::radians(transform.rotation.y);
        v3 move_input_rotated {
            move_input.x * glm::cos(rot_y) - move_input.y * glm::sin(rot_y),
            m_input->get_axis("fly"),
            move_input.x * glm::sin(rot_y) + move_input.y * glm::cos(rot_y) };

        transform.position += move_input_rotated * delta_time;
    }
}