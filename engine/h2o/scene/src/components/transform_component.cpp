#include  "scene/components/transform_component.h"

namespace h2o
{
    TransformComponent::TransformComponent(const ComponentInitializer& component_initializer)
        : Component(component_initializer)
    {}

    m4 TransformComponent::model_matrix() const
    {
        m4 model(1.0f);

        model = glm::scale(model, scale);
        model = glm::rotate(model, rotation.x, {1.0f, 0.0f, 0.0f});
        model = glm::rotate(model, rotation.y, {0.0f, 1.0f, 0.0f});
        model = glm::rotate(model, rotation.z, {0.0f, 0.0f, 1.0f});
        model = glm::translate(model, position);

        return model;
    }
}