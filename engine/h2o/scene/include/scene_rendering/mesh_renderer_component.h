#pragma once

#include "scene/component.h"

#include <string>

namespace h2o
{
    namespace gfx
    {
        class Texture;
        class Mesh;
    }

    class MeshRendererComponent : public Component
    {
    public:

        explicit MeshRendererComponent(const ComponentInitializer& component_initializer);
        ~MeshRendererComponent() override;

        void set_mesh(const std::shared_ptr<gfx::Mesh>& mesh);
        void set_texture(const std::shared_ptr<gfx::Texture>& texture);

        [[nodiscard]] const std::shared_ptr<gfx::Texture>& texture() const { return m_texture; }
        [[nodiscard]] const std::shared_ptr<gfx::Mesh>& mesh() const { return m_mesh; }

    private:

        std::shared_ptr<gfx::Texture> m_texture = nullptr;
        std::shared_ptr<gfx::Mesh> m_mesh = nullptr;

    };

    struct MeshRenderer
    {
        // Serialized
        std::string mesh_path{};
        std::string texture_path{};

        // Runtime only
        // TODO: Split mesh and texture into separate components?
        std::shared_ptr<gfx::Texture> texture = nullptr;
        std::shared_ptr<gfx::Mesh> mesh = nullptr;

        template<typename S>
        void serialize(S& s)
        { s(mesh_path, texture_path); }
    };
}