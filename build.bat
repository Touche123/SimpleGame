@echo off
clang -g ^
    -D_CRT_SECURE_NO_WARNINGS ^
    src/glad.c ^
    src/main.c ^
    src/list.c ^
    src/shader.c ^
    src/renderer_opengl.c ^
    src/mesh.c ^
    src/platform.c ^
    src/asset_manager.c ^
    src/math.c ^
    src/model.c ^
    src/scene.c ^
    src/io.c ^
    src/camera.c ^
    -Ideps/GLFW ^
    -IKHR ^
    -Ldeps/GLFW ^
    -lglfw3dll ^
    -lopengl32 ^
    -nodefaultlibs ^
    -o main.exe

@echo Compiled successfully
