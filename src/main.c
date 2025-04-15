#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "math.h"
#include "camera.h"
#include "asset_manager.h"
#include "defines.h"
#include "glad.h"
#include "glfw3.h"
#include "io.h"
#include "list.h"
#include "mat4.h"
#include "mesh.h"
#include "model.h"
#include "platform.h"
#include "renderer_opengl.h"
#include "shader.h"
#include "entity.h"
#include "scene.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

struct SomeStruct {
    int id;
    char *name;
};

typedef struct
{
    int id;
} entity;

typedef struct FrameData {
    float delta_time;
    float last_frame;
} FrameData;

void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
float direction[3] = {0.0f, 0.0f, 0.0f};
Camera camera = {0};
FrameData frame_data = {0};

bool keys[1024] = {false};
float last_x = 400, last_y = 300;  // mitten av skärmen (eller init efter glfwGetWindowSize)
bool first_mouse = true;

int main() {
    struct list_Node *intList = NULL;
    struct list_Node *floatList = NULL;
    struct list_Node *someStructs = NULL;

    entity *entityList = NULL;
    entity entityListArry[10];

    entityListArry[0].id = 2;

    int a = 10, b = 20, c = 30;
    list_append(&intList, &a);
    list_append(&intList, &b);
    list_append(&intList, &c);
    list_printIntList(intList);
    list_deleteFirstNode(&intList);

    GLFWwindow *window;

    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!renderer_opengl_init(SCREEN_WIDTH, SCREEN_HEIGHT)) {
        fprintf(stderr, "Could not initialize renderer.\n");
        return -1;
    }

    ShaderAsset* shaderAsset = asset_get_shader("default");
    printf("Shader asset id: %i\n", shaderAsset->shader.id);

    //Shader shader = {0}; 
    //shader.vertex_shader_filename = "res/shaders/vertex.glsl";
    //shader.fragment_shader_filename = "res/shaders/fragment.glsl";
    //    
    //shader.id = shader_compile(shader.vertex_shader_filename, shader.fragment_shader_filename);

    Scene scene = {0};
    scene_init(&scene);

    Model my_model = {0};
    model_load(&my_model, "res/models/cube.gltf");

    Entity *test_entity = scene_create_entity(&scene, &my_model);
    test_entity->transform.position[0] = -5.0f;
    test_entity->transform.scale[0] = 1.f;
    test_entity->transform.scale[1] = 1.f;
    test_entity->transform.scale[2] = 1.f;
    Model my_planemodel = {0};
    model_load(&my_planemodel, "res/models/plane.gltf");

    Entity *test_entity2 = scene_create_entity(&scene, &my_planemodel);
    test_entity2->transform.position[0] = 5.0f;

    float view[16], projection[16];

    mat4_translate(view, 0.0f, 0.0f, 0.0f);
    mat4_perspective(
            projection, 
            TO_RADIANS(45.0f),
            800.0f / 600.0f,
            0.1f, 1000.0f);

    float light_direction[] = {-1.0f, -1.0f, -1.0f};
    float light_color[] = {1.0f, 1.0f, 1.0f};

    unsigned char white_pixel[] = {255, 255, 255, 255};

    glEnable(GL_DEPTH_TEST);

    camera_init(&camera, 45.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    HANDLE file_changed = platform_register_watch_directory("res/shaders"); 
    HANDLE file_material_changed = platform_register_watch_directory("res"); 

    while (!glfwWindowShouldClose(window)) {
        // processInput(window);
        float current_frame = glfwGetTime();
        frame_data.delta_time = current_frame - frame_data.last_frame;
        frame_data.last_frame = current_frame;
       
        if (platform_watch_event(file_changed)) {
            printf("File changed! Reloading shader...\n");
            printf("Shader id: %i", shaderAsset->shader.id);
            shader_recompile(&shaderAsset->shader);
        }

        if (platform_watch_event(file_material_changed)) {
            printf("Material changed! Reloading shader...\n");

            for (size_t m = 0; m < test_entity->model->mesh_count; m++) {
                if(model_load_cached_material_from_file("res/Material.001", 
                            &test_entity->model->meshes[m].material)) {
                    
                }
            }
            shader_recompile(&shaderAsset->shader);
        }

        if (keys[GLFW_KEY_W])
            camera_process_keyboard(&camera, CAMERA_FORWARD, frame_data.delta_time);
        if (keys[GLFW_KEY_S])
            camera_process_keyboard(&camera, CAMERA_BACKWARD, frame_data.delta_time);
        if (keys[GLFW_KEY_A])
            camera_process_keyboard(&camera, CAMERA_LEFT, frame_data.delta_time);
        if (keys[GLFW_KEY_D])
            camera_process_keyboard(&camera, CAMERA_RIGHT, frame_data.delta_time);
        if (keys[GLFW_KEY_E] || keys[GLFW_KEY_SPACE])
            camera_process_keyboard(&camera, CAMERA_UP, frame_data.delta_time);
        if (keys[GLFW_KEY_Q] || keys[GLFW_KEY_LEFT_CONTROL])
            camera_process_keyboard(&camera, CAMERA_DOWN, frame_data.delta_time);
        if (keys[GLFW_KEY_T])
            shader_recompile(&shaderAsset->shader);
        // camera_update(&camera, delta_time);  // Update camera position and orientation
        float center[3] = {
            camera.position[0] + camera.front[0],
            camera.position[1] + camera.front[1],
            camera.position[2] + camera.front[2]};
        mat4_lookat(view, camera.position, center, camera.up);
        // mat4_translate(view, camera.position[0], camera.position[1], camera.position[2]);
        //   render
        //   ------
        renderer_opengl_prepare_frame();
        // TODO: This is make it so the shader id = 0. Why?
        ShaderAsset* testAsset = asset_get_shader("default");
        glUseProgram(testAsset->shader.id);

        GLuint loc_model = glGetUniformLocation(shaderAsset->shader.id, "model");
        GLuint loc_view = glGetUniformLocation(shaderAsset->shader.id, "view");
        GLuint loc_proj = glGetUniformLocation(shaderAsset->shader.id, "projection");
        GLuint loc_lightDir = glGetUniformLocation(shaderAsset->shader.id, "lightDir");
        GLuint loc_lightColor = glGetUniformLocation(shaderAsset->shader.id, "lightColor");
        GLuint loc_objectColor = glGetUniformLocation(shaderAsset->shader.id, "objectColor");
        glUniform3fv(loc_lightDir, 1, light_direction);
        glUniform3fv(loc_lightColor, 1, light_color);

        float rotation[16];
        float translation[16];

        glUniformMatrix4fv(loc_view, 1, GL_FALSE, view);
        glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection);

        for (size_t i = 0; i < scene.count; i++) {
            Entity *e = &scene.entities[i];

            float model_matrix[16];
            mat4_from_transform(model_matrix, &e->transform);
            //mat4_scale_in_place(model_matrix, 5.0f, 5.0f, 5.0f);
            mat4_identity(rotation);
            mat4_identity(translation);

            mat4_rotate_xyz(rotation, current_frame, current_frame * 0.5f, current_frame * 0.5f);
            mat4_mul(model_matrix, rotation, model_matrix);
            glUniformMatrix4fv(loc_model, 1, GL_FALSE, model_matrix);

            for (size_t m = 0; m < e->model->mesh_count; m++) {
                Mesh *mesh = &e->model->meshes[m];
                glUniform3fv(loc_objectColor, 1, mesh->material.base_color_factor);
                renderer_opengl_render_mesh(mesh);
            }
        }

        // draw our first triangle
        // glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// void processInput(GLFWwindow *window) {
//     if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, 1);
// }

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    static float sensitivity = 0.1f;

    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float x_offset = xpos - last_x;
    float y_offset = last_y - ypos;  // y går uppåt i OpenGL
    last_x = xpos;
    last_y = ypos;

    x_offset *= sensitivity;
    y_offset *= sensitivity;

    camera_process_mouse_movement(&camera, x_offset, y_offset);  // eller din egen kamera-instans
}
