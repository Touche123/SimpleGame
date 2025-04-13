#include "camera.h"
#include "math.h"

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
}

void camera_update(Camera* camera, float delta_time) {
    camera_update_vectors(camera);
    // // Update camera vectors based on yaw and pitch
    // float front[3];
    // front[0] = cosf(camera->yaw * (M_PI / 180.0f)) * cosf(camera->pitch * (M_PI / 180.0f));
    // front[1] = sinf(camera->pitch * (M_PI / 180.0f));
    // front[2] = sinf(camera->yaw * (M_PI / 180.0f)) * cosf(camera->pitch * (M_PI / 180.0f));

    // camera->front[0] = front[0];
    // camera->front[1] = front[1];
    // camera->front[2] = front[2];

    // // Recalculate the right and up vectors
    // camera->right[0] = camera->front[1] * camera->world_up[2] - camera->front[2] * camera->world_up[1];
    // camera->right[1] = camera->front[2] * camera->world_up[0] - camera->front[0] * camera->world_up[2];
    // camera->right[2] = camera->front[0] * camera->world_up[1] - camera->front[1] * camera->world_up[0];

    // // Normalize the right vector
    // float length = sqrtf(camera->right[0] * camera->right[0] + camera->right[1] * camera->right[1] + camera->right[2] * camera->right[2]);
    // if (length > 0.00001f) {
    //     camera->right[0] /= length;
    //     camera->right[1] /= length;
    //     camera->right[2] /= length;
    // }

    // // Calculate the up vector
    // camera->up[0] = camera->right[1] * front[2] - camera->right[2] * front[1];
    // camera->up[1] = camera->right[2] * front[0] - camera->right[0] * front[2];
    // camera->up[2] = camera->right[0] * front[1] - camera->right[1] * front[0];

    // // Normalize the up vector
    // length = sqrtf(camera->up[0] * camera->up[0] + camera->up[1] * camera->up[1] + camera->up[2] * camera->up[2]);
    // if (length > 0.00001f) {
    //     camera->up[0] /= length;
    //     camera->up[1] /= length;
    //     camera->up[2] /= length;
    // }
}

void camera_process_keyboard(Camera* camera, Camera_Movement direction, float delta_time) {
    float camera_speed = 25.0f * delta_time;  // Adjust accordingly
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
    camera->yaw += x_offset;
    camera->pitch += y_offset;

    // Begränsa pitch för att undvika "flip"
    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    // Uppdatera kamerans front/right/up vektorer
    camera_update_vectors(camera);
}

void camera_update_vectors(Camera* camera) {
    float yaw_rad = TO_RADIANS(camera->yaw);
    float pitch_rad = TO_RADIANS(camera->pitch);

    // Uppdatera front-vektor
    camera->front[0] = cosf(yaw_rad) * cosf(pitch_rad);
    camera->front[1] = sinf(pitch_rad);
    camera->front[2] = sinf(yaw_rad) * cosf(pitch_rad);
    vec3_normalize(camera->front);

    // Right = cross(front, world_up)
    vec3_cross(camera->front, camera->world_up, camera->right);
    vec3_normalize(camera->right);

    // Up = cross(right, front)
    vec3_cross(camera->right, camera->front, camera->up);
    vec3_normalize(camera->up);
}
