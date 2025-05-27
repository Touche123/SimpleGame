#ifndef CAMERA_H
#define CAMERA_H
#include <cglm/cglm.h>

#include "core.h"

typedef struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 world_up;
    f32 yaw;
    f32 pitch;
    f32 fov;
    f32 speed;
    f32 aspect_ratio;
    f32 near_plane;
    f32 far_plane;
    mat4 projection_matrix;
    mat4 view_matrix;
} Camera;

typedef enum {
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
} Camera_Movement;

void camera_init(Camera* camera, float fov, float aspect_ratio, float near_plane, float far_plane);
void camera_process_keyboard(Camera* camera, Camera_Movement cameraMovement, float delta_time);
void camera_process_mouse_movement(Camera* camera, float x_offset, float y_offset);
void camera_update_vectors(Camera* camera);
// void camera_process_mouse_scroll(Camera* camera, float y_offset);
const float* camera_get_view_matrix(Camera* camera);
void camera_set_mouse_cursor_enabled(bool enabled);
const float* camera_get_projection_matrix(const Camera* camera);
void camera_update(Camera* camera, float dt);
// void camera_get_position(Camera* camera, float* position);
// void camera_get_front(Camera* camera, float* front);
// void camera_get_up(Camera* camera, float* up);
// void camera_get_right(Camera* camera, float* right);
// void camera_get_yaw(Camera* camera, float* yaw);
// void camera_get_pitch(Camera* camera, float* pitch);
// void camera_get_fov(Camera* camera, float* fov);
void camera_set_position(Camera* camera, float x, float y, float z);
// void camera_set_front(Camera* camera, float x, float y, float z);
// void camera_set_up(Camera* camera, float x, float y, float z);
// void camera_set_right(Camera* camera, float x, float y, float z);
// void camera_set_yaw(Camera* camera, float yaw);
// void camera_set_pitch(Camera* camera, float pitch);
// void camera_set_fov(Camera* camera, float fov);
// void camera_set_world_up(Camera* camera, float x, float y, float z);
// void camera_set_aspect_ratio(Camera* camera, float aspect_ratio);
// void camera_set_near_plane(Camera* camera, float near_plane);
// void camera_set_far_plane(Camera* camera, float far_plane);
// void camera_set_view_matrix(Camera* camera, float* view_matrix);
// void camera_set_projection_matrix(Camera* camera, float* projection_matrix, float aspect_ratio, float near_plane, float far_plane);
// void camera_set_viewport(Camera* camera, int width, int height);

#endif
