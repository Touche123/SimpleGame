// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
// #include "math.h"
#include "camera.h"
// #include "core.h"
// #include "glad.h"
// #include "os/os_glfw3.h"
#include <cglm/cglm.h>

#include "asset_manager.h"
#include "gpu/gpu_backend_opengl.h"
#include "os/os_windows.h"
// #define GLFW_INCLUDE_VULKAN
// #include <glfw3.h>
// #include "io.h"
// #include "list.h"
// #include "os_windows.h"
// #include "mesh.h"
// #include "model.h"
// #include "platform.h"
// #include "gpu_backend_opengl.h"
// #include "gpu_backend_vulkan.h"
// #include "shader.h"
#include <mmsystem.h>

#include "entity.h"
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

typedef struct {
    vec3 position;
    vec3 color;
    float intensity;
} Light;

void pbr_materials(Mesh* mesh, Shader* shader) {
    // glUniform1i(glGetUniformLocation(shader->id, "useAlbedoMap"), mesh->material.albedoColor != 0);
    // glUniform1i(glGetUniformLocation(shader->id, "useMetallicMap"), mesh->material.metallicValue);
    // glUniform1i(glGetUniformLocation(shader->id, "useRoughnessMap"), mesh->material.roughnessValue != 0);
    // glUniform1i(glGetUniformLocation(shader->id, "useAoMap"), mesh->material.aoValue != 0);

    glUniform1i(glGetUniformLocation(shader->id, "useAlbedoMap"), mesh->material.albedoTexture ? 1 : 0);
    glUniform1i(glGetUniformLocation(shader->id, "useMetallicMap"), mesh->material.metallicTexture ? 1 : 0);
    glUniform1i(glGetUniformLocation(shader->id, "useRoughnessMap"), mesh->material.roughnessTexture ? 1 : 0);
    glUniform1i(glGetUniformLocation(shader->id, "useAoMap"), mesh->material.aoTexture ? 1 : 0);
    glUniform1i(glGetUniformLocation(shader->id, "useAoMap"), mesh->material.useNormalMap ? 1 : 0);
    // printf("albedoColor: %f, %f, %f\n", mesh->material.albedoColor[0], mesh->material.albedoColor[1], mesh->material.albedoColor[2]);
    glUniform3fv(glGetUniformLocation(shader->id, "albedoColor"), 1, mesh->material.albedoColor);
    glUniform1f(glGetUniformLocation(shader->id, "metallicValue"), mesh->material.metallicValue);
    glUniform1f(glGetUniformLocation(shader->id, "roughnessValue"), mesh->material.roughnessValue);
    glUniform1f(glGetUniformLocation(shader->id, "aoValue"), mesh->material.aoValue);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, mesh->material.albedoTexture);
    // glUniform1i(glGetUniformLocation(shader->id, "albedoMap"), 0);

    if (mesh->material.albedoTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->material.albedoTexture);
        glUniform1i(glGetUniformLocation(shader->id, "albedoMap"), 0);
    }
    if (mesh->material.metallicTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mesh->material.metallicTexture);
        glUniform1i(glGetUniformLocation(shader->id, "metallicMap"), 1);
    }
    if (mesh->material.roughnessTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mesh->material.roughnessTexture);
        glUniform1i(glGetUniformLocation(shader->id, "roughnessMap"), 2);
    }
    if (mesh->material.aoTexture ? 1 : 0) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mesh->material.aoTexture);
        glUniform1i(glGetUniformLocation(shader->id, "aoMap"), 3);
    }
    if (mesh->material.useNormalMap) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mesh->material.normalTexture);
        glUniform1i(glGetUniformLocation(shader->id, "normalMap"), 4);
        glUniform1i(glGetUniformLocation(shader->id, "useNormalMap"), 1);
    } else {
        glUniform1i(glGetUniformLocation(shader->id, "useNormalMap"), 0);
    }
}

f32 direction[3] = {0.0f, 0.0f, 0.0f};
// Camera camera = {0};

bool keys[1024] = {false};
bool first_mouse = true;
double fps_timer = 0.0;
int frame_count = 0;
double fps = 0.0;

int game_main() {
    // gpu_backend_vulkan_init();
    os_init();
    OsWindowId window_id = os_window_open(800, 600);
    gpu_backend_opengl_init(800, 600);

    ShaderAsset* shaderAsset = asset_get_shader("default");
    ShaderAsset* shaderAsset_brdf = asset_get_shader("brdf_lut");

    Scene scene = {0};
    scene_init(&scene);

    // Model my_model = {0};
    // model_load(&my_model, "res/models/cube.gltf");

    // Entity* test_entity = scene_create_entity(&scene, &my_model);
    //  test_entity->transform.position[0] = -5.0f;
    //  test_entity->transform.scale[0] = 1.f;
    //  test_entity->transform.scale[1] = 1.f;
    //  test_entity->transform.scale[2] = 1.f;
    Model my_planemodel = {0};
    model_load(&my_planemodel, "res/models/plane.gltf");
    // Model my_testmodel = {0};
    // model_load(&my_testmodel, "res/models/testscene.gltf");
    Model model_barrel = {0};
    model_load(&model_barrel, "res/models/barrel/Barrel.gltf");

    Entity* test_entity2 = scene_create_entity(&scene, &my_planemodel);
    // Entity* test_entity3 = scene_create_entity(&scene, &my_planemodel);
    // Entity* test_entity4 = scene_create_entity(&scene, &my_testmodel);
    Entity* test_entity5 = scene_create_entity(&scene, &model_barrel);
    // test_entity2->transform.position[0] = 5.0f;
    // test_entity3->transform.position[0] = 0.0f;

    // test_entity3->transform.position[1] = -1.0f;
    // test_entity3->transform.scale[0] = 10.0f;
    // test_entity3->transform.scale[1] = 10.0f;
    // test_entity3->transform.scale[2] = 10.0f;

    mat4 view, projection;

    glm_mat4_identity(view);
    glm_perspective(TO_RADIANS(45.0f), (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 1000.0f, projection);

    vec3 light_direction[] = {-1.0f, -1.0f, -1.0f};
    vec3 light_color[] = {1.0f, 1.0f, 1.0f};

    vec4 white_pixel[] = {255, 255, 255, 255};

    glEnable(GL_DEPTH_TEST);
    Camera camera = {0};
    camera_init(&camera, 45.0f, (f32)SCREEN_WIDTH / (f32)SCREEN_HEIGHT, 0.1f, 100.0f);

    vec2 vector;
    glm_vec2_zero(vector);

    bool running = true;
    float rotation_angle_x = 0.0f;
    float rotation_angle_y = 0.0f;
    float rotation_angle_z = 0.0f;
    bool keys[256] = {0};
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
                    // printf("typed: %i\n", event.key);
                    break;
                case OS_EVENT_TYPE_KEY_PRESSED:
                    keys[event.key] = true;
                    if (event.key == 87) {
                        camera_process_keyboard(&camera, CAMERA_FORWARD, os_windows_get_delta_time());
                    } else if (event.key == 83) {
                        camera_process_keyboard(&camera, CAMERA_BACKWARD, os_windows_get_delta_time());
                    } else if (event.key == 65) {
                        camera_process_keyboard(&camera, CAMERA_LEFT, os_windows_get_delta_time());
                    } else if (event.key == 68) {
                        camera_process_keyboard(&camera, CAMERA_RIGHT, os_windows_get_delta_time());
                    } else if (event.key == 32 || event.key == 69) {
                        camera_process_keyboard(&camera, CAMERA_UP, os_windows_get_delta_time());
                    } else if (event.key == 17 || event.key == 81) {
                        camera_process_keyboard(&camera, CAMERA_DOWN, os_windows_get_delta_time());
                    }
                    // printf("key pressed: %i\n", event.key);
                    break;
                case OS_EVENT_TYPE_KEY_RELEASED:
                    keys[event.key] = false;
                    if (event.key == 27) {
                        mousecursor_enabled = !mousecursor_enabled;
                        os_mouse_show(mousecursor_enabled);
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

        if (keys[87]) {  // W
            camera_process_keyboard(&camera, CAMERA_FORWARD, os_windows_get_delta_time());
        }
        if (keys[83]) {  // S
            camera_process_keyboard(&camera, CAMERA_BACKWARD, os_windows_get_delta_time());
        }
        if (keys[65]) {  // A
            camera_process_keyboard(&camera, CAMERA_LEFT, os_windows_get_delta_time());
        }
        if (keys[68]) {  // D
            camera_process_keyboard(&camera, CAMERA_RIGHT, os_windows_get_delta_time());
        }
        if (keys[32] || keys[69]) {  // SPACE or E
            camera_process_keyboard(&camera, CAMERA_UP, os_windows_get_delta_time());
        }
        if (keys[17] || keys[81]) {  // CTRL or Q
            camera_process_keyboard(&camera, CAMERA_DOWN, os_windows_get_delta_time());
        }

        if (keys[16]) {  // D
            camera.speed = 25.0f;
        } else {
            camera.speed = 5.0f;
        }

        vec3 center = {
            camera.position[0] + camera.front[0],
            camera.position[1] + camera.front[1],
            camera.position[2] + camera.front[2]};

        glm_lookat(camera.position, center, camera.up, view);
        // mat4_lookat(view, camera.position, center, camera.up);

        gpu_backend_opengl_prepare_frame();
        // TODO: This is make it so the shader id = 0. Why?
        ShaderAsset* testAsset = asset_get_shader("default");
        shader_use(&testAsset->shader.id);

        Light light = {
            .position = {5.0f, 5.0f, 5.0f},
            .color = {1.0f, 1.0f, 1.0f},
            .intensity = 500.0f,
        };

        shader_uniform3fv(&shaderAsset->shader, "lightPos", light.position);
        shader_uniform3fv(&shaderAsset->shader, "lightColor", light.color);
        shader_uniform1f(&shaderAsset->shader, "lightIntensity", light.intensity);
        shader_uniform3fv(&shaderAsset->shader, "camPos", camera.position);
        shader_uniform1f(&shaderAsset->shader, "exposure", 1.0f);

        // float rotation[16];
        mat4 rotation, scale;
        // float scale[16];
        shader_uniformMatrix4fv(&shaderAsset->shader, "view", view[0]);
        shader_uniformMatrix4fv(&shaderAsset->shader, "projection", projection[0]);

        rotation_angle_x += 1.0f * os_windows_get_delta_time();  // 1 radian per sekund
        rotation_angle_y += 0.5f * os_windows_get_delta_time();  // 0.5 radian per sekund
        rotation_angle_z += 0.5f * os_windows_get_delta_time();  // 0.5 radian per sekund
        if (rotation_angle_x > 2.0f * 3.14159265f) rotation_angle_x -= 2.0f * 3.14159265f;
        if (rotation_angle_y > 2.0f * 3.14159265f) rotation_angle_y -= 2.0f * 3.14159265f;
        if (rotation_angle_z > 2.0f * 3.14159265f) rotation_angle_z -= 2.0f * 3.14159265f;

        int scene_idx;
        for_range(scene_idx, 0, scene.count) {
            Entity* e = &scene.entities[scene_idx];
            mat4 model_matrix;

            glm_mat4_identity(model_matrix);
            glm_mat4_identity(rotation);
            glm_mat4_identity(scale);
            glm_scale(scale, e->transform.scale);
            glm_translate(model_matrix, e->transform.position);
            glm_mul(model_matrix, scale, model_matrix);

            shader_use(&testAsset->shader.id);
            shader_uniformMatrix4fv(&shaderAsset->shader, "model", model_matrix[0]);
            gpu_backend_opengl_set_framebuffer_size(800, 600);
            for (size_t m = 0; m < e->model->mesh_count; m++) {
                Mesh* mesh = &e->model->meshes[m];

                pbr_materials(mesh, &shaderAsset->shader);
                // glUniform3fv(loc_objectColor, 1, mesh->material.albedoColor);
                gpu_backend_opengl_render_mesh(mesh);
            }
        }
        gpu_backend_opengl_end_frame();
    }
    return 0;
}
