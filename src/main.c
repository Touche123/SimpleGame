#include <cglm/cglm.h>

#include "asset_manager.h"
#include "camera.h"
#include "entity.h"
#include "gpu/gpu_backend_opengl.h"
#include "io/io_binary_streamreader.h"
#include "io/io_binary_streamwriter.h"
#include "os/os_windows.h"
#include "scene.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

struct SomeStruct {
    s32 id;
    char* name;
};

typedef struct {
    s32 id;
} entity;

f32 direction[3] = {0.0f, 0.0f, 0.0f};
bool keys[1024] = {false};
bool first_mouse = true;
double fps_timer = 0.0;
int frame_count = 0;
double fps = 0.0;

int game_main() {
    os_init();
    OsWindowId window_id = os_window_open(800, 600);
    gpu_backend_opengl_init(800, 600);

    ShaderAsset* shaderAsset = asset_get_shader("default");
    ShaderAsset* shaderAsset_brdf = asset_get_shader("brdf_lut");

    Scene scene = {0};
    scene_init(&scene);

    ModelAsset* modelAsset_plane = asset_get_model("res/models/plane.gltf");
    ModelAsset* modelAsset_barrel = asset_get_model("res/models/barrel/Barrel.gltf");

    Entity* test_entity2 = scene_create_entity(&scene, &modelAsset_plane->model);
    Entity* test_entity5 = scene_create_entity(&scene, &modelAsset_barrel->model);

    Light* point_light = scene_create_light(&scene, (vec3){5.0f, 5.0f, 5.0f}, (vec3){1.0f, 1.0f, 1.0f}, 500.0f);
    Light* point_light2 = scene_create_light(&scene, (vec3){10.0f, 0.0f, 5.0f}, (vec3){1.0f, 0.0f, 0.0f}, 500.0f);

    Camera camera = {0};
    camera_init(&camera, 45.0f, (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 100.0f);

    bool running = true;
    bool mousecursor_enabled = false;
    camera.speed = 2.5f;

    while (running) {
        os_windows_update_delta_time();

        OsEvent event;
        while (os_events_iter_next(&event)) {
            switch (event.type) {
                case OS_EVENT_TYPE_UNKNOWN:
                    break;
                case OS_EVENT_TYPE_KEY_TYPED:
                    break;
                case OS_EVENT_TYPE_KEY_PRESSED:
                    keys[event.key] = true;
                    break;
                case OS_EVENT_TYPE_KEY_RELEASED:
                    keys[event.key] = false;
                    if (event.key == OS_KEY_ESCAPE) {
                        mousecursor_enabled = !mousecursor_enabled;
                        os_mouse_show(mousecursor_enabled);
                        camera_set_mouse_cursor_enabled(!mousecursor_enabled);
                        break;
                    }
                    break;
                case OS_EVENT_TYPE_WINDOW_RESIZED:
                    printf("window resize - width: %i, height: %i\n", event.window_size.window_width, event.window_size.window_height);
                    break;
                case OS_EVENT_TYPE_MOUSE_MOVED:
                    float sensitivity = 0.1f;
                    float x_offset = (float)event.mouse_move.mouse_delta_x * sensitivity;
                    float y_offset = (float)event.mouse_move.mouse_delta_y * sensitivity;

                    camera_process_mouse_movement(&camera, x_offset, y_offset);
                    break;
                case OS_EVENT_TYPE_WINDOW_CLOSED:
                    exit(0);
                    break;
            }
        }

        if (keys[OS_KEY_W]) {
            camera_process_keyboard(&camera, CAMERA_FORWARD, os_windows_get_delta_time());
        }
        if (keys[OS_KEY_S]) {
            camera_process_keyboard(&camera, CAMERA_BACKWARD, os_windows_get_delta_time());
        }
        if (keys[OS_KEY_A]) {
            camera_process_keyboard(&camera, CAMERA_LEFT, os_windows_get_delta_time());
        }
        if (keys[OS_KEY_D]) {
            camera_process_keyboard(&camera, CAMERA_RIGHT, os_windows_get_delta_time());
        }
        if (keys[OS_KEY_SPACE] || keys[OS_KEY_E]) {
            camera_process_keyboard(&camera, CAMERA_UP, os_windows_get_delta_time());
        }
        if (keys[OS_KEY_LCTRL] || keys[OS_KEY_Q]) {
            camera_process_keyboard(&camera, CAMERA_DOWN, os_windows_get_delta_time());
        }

        if (keys[OS_KEY_LSHIFT]) {
            camera.speed = 25.0f;
        } else {
            camera.speed = 5.0f;
        }

        camera_update(&camera, os_windows_get_delta_time());
        gpu_backend_opengl_prepare_frame();
        ShaderAsset* testAsset = asset_get_shader("default");
        shader_use(&testAsset->shader.id);

        shader_uniform3fv(&shaderAsset->shader, "camPos", camera.position);
        shader_uniform1f(&shaderAsset->shader, "exposure", 1.0f);

        shader_uniformMatrix4fv(&shaderAsset->shader, "view", camera_get_view_matrix(&camera));
        shader_uniformMatrix4fv(&shaderAsset->shader, "projection", camera_get_projection_matrix(&camera));

        int scene_light_idx;
        shader_uniform1i(&shaderAsset->shader, "lightCount", scene.light_count);

        for_range(scene_light_idx, 0, scene.light_count) {
            Light* light = &scene.lights[scene_light_idx];
            char name[64];

            sprintf(name, "lights[%d].position", scene_light_idx);
            shader_uniform3fv(&shaderAsset->shader, name, light->position);

            sprintf(name, "lights[%d].color", scene_light_idx);
            shader_uniform3fv(&shaderAsset->shader, name, light->color);

            sprintf(name, "lights[%d].intensity", scene_light_idx);
            shader_uniform1f(&shaderAsset->shader, name, light->intensity);
        }

        int scene_entity_idx;
        for_range(scene_entity_idx, 0, scene.entity_count) {
            Entity* e = &scene.entities[scene_entity_idx];
            mat4 model_matrix;
            entity_get_model_matrix(e, &model_matrix);
            shader_use(&testAsset->shader.id);
            shader_uniformMatrix4fv(&shaderAsset->shader, "model", model_matrix[0]);
            gpu_backend_opengl_set_framebuffer_size(800, 600);
            for (size_t m = 0; m < e->model->mesh_count; m++) {
                Mesh* mesh = &e->model->meshes[m];
                mesh_apply_material(mesh, &shaderAsset->shader);
                gpu_backend_opengl_render_mesh(mesh);
            }
        }
        gpu_backend_opengl_end_frame();
    }
    return 0;
}
