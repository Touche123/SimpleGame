#include "gpu_backend_opengl.h"

#include "../mesh.h"
#include "../os/os_windows.h"
#include "../shader.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0X2092
#define WGL_CONTEXT_FLAGS_ARB 0X2094
#define WGL_CONTEXT_COREPROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int* attribList);
typedef const char*(WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef int(WINAPI* PFNWGLGETSWAPINTERVALEXTPROC)(void);

void glCheckError(const char* file, int line) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        const char* errorStr = "UNKNOWN";
        switch (error) {
            case GL_INVALID_ENUM:
                errorStr = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorStr = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                errorStr = "GL_INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                errorStr = "GL_OUT_OF_MEMORY";
                break;
        }
        printf("[OpenGL ERROR] %s - %s:%d\n", errorStr, file, line);
    }
}
#define GL_CHECK() glCheckError(__FILE__, __LINE__)

int gpu_backend_opengl_init(int width, int height) {
    OsWindowId iw;

    OsWindowBackend window = os_window_backend(iw);

    HDC hdc = GetDC(window.window);
    PIXELFORMATDESCRIPTOR pfd = {0};
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 32;
    pfd.cStencilBits = 8;

    int pixel_format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);

    HGLRC tempRC = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempRC);
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    const int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,
        3,
        WGL_CONTEXT_MINOR_VERSION_ARB,
        3,
        WGL_CONTEXT_FLAGS_ARB,
        0,
        WGL_CONTEXT_PROFILE_MASK_ARB,
        WGL_CONTEXT_COREPROFILE_BIT_ARB,
        0,
    };

    HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempRC);
    wglMakeCurrent(hdc, hglrc);

    if (!gladLoadGL()) {
        printf("Could not initialize GLAD \n");
    } else {
        printf("OpenGL Version  %i\n", GLVersion.major);
    }

    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
    bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;
    // https://www.khronos.org/opengl/wiki/Swap_Interval
    int vsynch = 0;

    if (swapControlSupported) {
        PFNWGLSWAPINTERVALEXTPROC wglSwapInternalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if (wglSwapInternalEXT(1)) {
            printf("VSynch enabled \n");
        } else {
            printf("Could not enable VSynch");
        }
    } else {
        printf("WGL_EXT_swap_control not supported \n");
    }

    // Initialize OpenGL context and set viewport
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);                            // Enable depth testing for 3D rendering
    glEnable(GL_BLEND);                                 // Enable blending for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Set blend function
    return 1;
}

void gpu_backend_opengl_render_mesh(Mesh* mesh) {
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    // GL_CHECK();
}

void gpu_backend_opengl_prepare_frame() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gpu_backend_opengl_end_frame() {
    // Swap buffers and poll events
    OsWindowId iw;
    OsWindowBackend window = os_window_backend(iw);
    HDC hdc = GetDC(window.window);
    SwapBuffers(hdc);
    ReleaseDC(window.window, hdc);
}

GLuint gpu_backend_opengl_generate_brdf_lut(Shader* shader) {
    GLuint brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader->id);  // En shader som beräknar integralen
    gpu_backend_opengl_renderFullscreenQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return brdfLUTTexture;
}

void gpu_backend_opengl_set_framebuffer_size(int width, int height) {
    glViewport(0, 0, width, height);
}

void gpu_backend_opengl_renderFullscreenQuad() {
    static GLuint quadVAO = 0;
    static GLuint quadVBO;

    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions
            -1.0f,
            -1.0f,
            1.0f,
            -1.0f,
            -1.0f,
            1.0f,
            1.0f,
            1.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
