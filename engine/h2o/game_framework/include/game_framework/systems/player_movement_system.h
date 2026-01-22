#pragma once

#include "input/input_module.h"
#include "scene/scene_system.h"

namespace h2o
{
    struct PlayerMovementComp;

    class PlayerMovementSystem : public SceneSystem
    {
    public:

        explicit PlayerMovementSystem(const SceneSystemInitializer& system_initializer);

        void update(f32 delta_time) override;

    private:

        [[nodiscard]] v3 get_desired_move_dir(f32 rot_y) const;
        [[nodiscard]] bool touching_grass() const;
        [[nodiscard]] v3 steered_horizontal_movement(const v3& velocity, f32 accel, f32 decel, f32 max_speed, const f32 rot_y, f32 delta_time) const;

        void update_fly (f32 delta_time, const PlayerMovementComp& movement_comp, bool is_sprinting, f32 rot_y, v3& velocity) const;
        void update_walk(f32 delta_time, const PlayerMovementComp& movement_comp, bool is_sprinting, f32 rot_y, v3& velocity) const;
        void update_fall(f32 delta_time, const PlayerMovementComp& movement_comp, bool is_sprinting, f32 rot_y, v3& velocity) const;

    private:

        InputModule& m_input_module;

    };
}
