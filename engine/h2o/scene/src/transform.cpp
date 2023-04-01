#include "scene/transform.h"

namespace h2o
{
    m4 Transform::model_matrix() const
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