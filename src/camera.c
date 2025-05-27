#include "camera.h"

#include "math.h"

static bool mousecursor_enabled = true;

void camera_init(Camera* camera, float fov, float aspect_ratio, float near_plane, float far_plane) {
    camera->position[0] = 0.0f;
    camera->position[1] = 0.0f;
    camera->position[2] = 20.0f;

    camera->front[0] = 0.0f;
    camera->front[1] = 0.0f;
    camera->front[2] = -1.0f;

    camera->up[0] = 0.0f;
    camera->up[1] = 1.0f;
    camera->up[2] = 0.0f;

    camera->right[0] = 1.0f;
    camera->right[1] = 0.0f;
    camera->right[2] = 0.0f;

    camera->world_up[0] = 0.0f;
    camera->world_up[1] = 1.0f;
    camera->world_up[2] = 0.0f;

    camera->yaw = -90.0f;  // Yaw is initialized to -90 degrees
    camera->pitch = 0.0f;  // Pitch is initialized to 0 degrees
    camera->fov = fov;

    camera->aspect_ratio = aspect_ratio;
    camera->near_plane = near_plane;
    camera->far_plane = far_plane;
}

void camera_set_mouse_cursor_enabled(bool enabled) {
    mousecursor_enabled = enabled;
}

const float* camera_get_view_matrix(Camera* camera) {
    return (const float*)camera->view_matrix;
}

const float* camera_get_projection_matrix(const Camera* camera) {
    return (const float*)camera->projection_matrix;
    // glm_perspective(TO_RADIANS(camera->fov), camera->aspect_ratio, camera->near_plane, camera->far_plane, projection_matrix);
}

void camera_update(Camera* camera, float dt) {
    // Update the projection matrix based on the current camera parameters
    glm_perspective(TO_RADIANS(camera->fov), camera->aspect_ratio, camera->near_plane, camera->far_plane, camera->projection_matrix);
    float center[3] = {
        camera->position[0] + camera->front[0],
        camera->position[1] + camera->front[1],
        camera->position[2] + camera->front[2]};

    glm_lookat(camera->position, center, camera->up, camera->view_matrix);

    // Update the view matrix based on the current camera position and orientation
    // camera_get_view_matrix(camera, camera->view_matrix);
}

void camera_process_keyboard(Camera* camera, Camera_Movement direction, float dt) {
    float camera_speed = camera->speed * dt;

    if (direction == CAMERA_FORWARD) {
        camera->position[0] += camera->front[0] * camera_speed;
        camera->position[1] += camera->front[1] * camera_speed;
        camera->position[2] += camera->front[2] * camera_speed;
    }
    if (direction == CAMERA_BACKWARD) {
        camera->position[0] -= camera->front[0] * camera_speed;
        camera->position[1] -= camera->front[1] * camera_speed;
        camera->position[2] -= camera->front[2] * camera_speed;
    }
    if (direction == CAMERA_LEFT) {
        camera->position[0] -= camera->right[0] * camera_speed;
        camera->position[1] -= camera->right[1] * camera_speed;
        camera->position[2] -= camera->right[2] * camera_speed;
    }
    if (direction == CAMERA_RIGHT) {
        camera->position[0] += camera->right[0] * camera_speed;
        camera->position[1] += camera->right[1] * camera_speed;
        camera->position[2] += camera->right[2] * camera_speed;
    }
    if (direction == CAMERA_UP) {
        camera->position[0] += camera->world_up[0] * camera_speed;
        camera->position[1] += camera->world_up[1] * camera_speed;
        camera->position[2] += camera->world_up[2] * camera_speed;
    }
    if (direction == CAMERA_DOWN) {
        camera->position[0] += camera->world_up[0] * camera_speed;
        camera->position[1] += camera->world_up[1] * -1 * camera_speed;
        camera->position[2] += camera->world_up[2] * camera_speed;
    }
}

void camera_set_position(Camera* camera, float x, float y, float z) {
    camera->position[0] = x;
    camera->position[1] = y;
    camera->position[2] = z;
}

void camera_process_mouse_movement(Camera* camera, float x_offset, float y_offset) {
    if (!mousecursor_enabled) {
        return;
    }

    camera->yaw += x_offset;
    camera->pitch += y_offset;

    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    camera_update_vectors(camera);
}

void camera_update_vectors(Camera* camera) {
    float yaw_rad = TO_RADIANS(camera->yaw);
    float pitch_rad = TO_RADIANS(camera->pitch);

    camera->front[0] = cosf(yaw_rad) * cosf(pitch_rad);
    camera->front[1] = sinf(pitch_rad);
    camera->front[2] = sinf(yaw_rad) * cosf(pitch_rad);
    glm_normalize(camera->front);

    glm_cross(camera->front, camera->world_up, camera->right);
    glm_normalize(camera->right);

    glm_cross(camera->right, camera->front, camera->up);
    glm_normalize(camera->up);
}
