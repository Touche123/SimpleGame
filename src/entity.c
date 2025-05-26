#include "entity.h"

void entity_get_model_matrix(Entity* e, mat4 out) {
    mat4 model_matrix, rotation, scale;

    glm_mat4_identity(model_matrix);
    glm_mat4_identity(rotation);
    glm_mat4_identity(scale);
    glm_scale(scale, e->transform.scale);
    glm_translate(model_matrix, e->transform.position);
    glm_mul(model_matrix, scale, model_matrix);
    glm_mat4_copy(model_matrix, out);
}