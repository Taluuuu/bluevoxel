#include "game_framework/actors/player_character.h"

#include "core/engine.h"
#include "game_framework/components/fps_camera_component.h"
#include "game_framework/components/player_movement_component.h"
#include "physics/scene/collider_component.h"
#include "physics/scene/physics_system.h"
#include "scene/scene.h"

#include <glm/gtx/norm.hpp>

namespace h2o
{
    PlayerCharacter::PlayerCharacter(const ActorInitializer& actor_initializer)
        : Actor(actor_initializer)
    {
        m_collider = add_component<ColliderComponent>();
        m_collider->set_size(v3{ 0.8f, 1.7f, 0.8f });
        m_collider->set_offset(v3{ -0.4f, -1.5f, -0.4f });

        add_component<FpsCameraComponent>();
        add_component<PlayerMovementComponent>();
    }
}
