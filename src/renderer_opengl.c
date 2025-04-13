#include "renderer_opengl.h"

#include "glad.h"
#include "glfw3.h"
#include "mesh.h"

int renderer_opengl_init(int width, int height) {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    // Initialize OpenGL context and set viewport
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);                            // Enable depth testing for 3D rendering
    glEnable(GL_BLEND);                                 // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Set blend function
    return 1;
}

void renderer_opengl_render_mesh(Mesh *mesh) {
    glBindTexture(GL_TEXTURE_2D, mesh->texture);
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}

void renderer_opengl_prepare_frame() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
