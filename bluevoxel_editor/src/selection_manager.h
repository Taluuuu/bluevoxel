#pragma once

#include "core/events.h"
#include "core/tickable.h"
#include "input/key_state.h"
#include "physics/primitives.h"
#include "physics/ray.h"

#include <functional>
#include <vector>

namespace h2o
{
    class InputModule;
    class RenderingModule;
    class WindowingModule;
}

namespace bluevoxel
{
    namespace phys = h2o::physics;

    struct HoverData
    {
        h2o::KeyState click_state{};
        f32 t = 0.0f;
    };

    using SelectionCallback = std::function<void(const HoverData&)>;

    template<class PrimitiveType>
    struct Selectable
    {
        PrimitiveType primitive{};
        SelectionCallback callback{};
    };

    class SelectionManager : public h2o::Tickable
    {
    public:

        explicit SelectionManager(h2o::Tickable* owner);
        ~SelectionManager() override = default;

        void pause_selection();
        void resume_selection();

        [[nodiscard]] const phys::Ray& mouse_ray() const { return m_mouse_ray; }

        template<class PrimitiveType>
        void add(const PrimitiveType& primitive, const SelectionCallback& callback);

        // TODO: Add some sort of add_persistent that does not require adding every frame

    public:

        struct OnClickedNothing{};
        h2o::Event<OnClickedNothing> on_clicked_nothing{};

    protected:

        void frame_start(f32 delta_time) override;
        void post_update(f32 delta_time) override;

    private:

        std::vector< Selectable<phys::Cylinder> > m_cylinders{};
        std::vector< Selectable<phys::Triangle> > m_triangles{};
        std::vector< Selectable<phys::Sphere> > m_spheres{};

        bool m_is_selection_enabled = true;
        phys::Ray m_mouse_ray{};

        // Module refs
        h2o::InputModule*     const m_input_module     = nullptr;
        h2o::RenderingModule* const m_rendering_module = nullptr;
        h2o::WindowingModule* const m_window_module    = nullptr;

    };
}