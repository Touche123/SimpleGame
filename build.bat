@echo off
clang -g -Wextra -Wfloat-conversion -D_CRT_SECURE_NO_WARNINGS ^
-DNOMINMAX ^
src/core.c ^
src/io/io_binary_streamreader.c ^
src/io/io_binary_streamwriter.c ^
src/scene.c ^
src/camera.c ^
src/glad.c ^
src/mesh.c ^
src/entity.c ^
src/model.c ^
src/shader.c ^
src/asset_manager.c ^
src/os/os_windows.c ^
src/gpu/gpu_backend_opengl.c ^
src/gpu/gpu_backend_vulkan.c ^
src/main.c ^

-Ideps ^
-Ideps/GLFW ^
-IKHR ^
-IC:\VulkanSDK\1.4.309.0\Include\ ^
-LC:\VulkanSDK\1.4.309.0\Lib\ ^
-Ldeps/GLFW ^
-lglfw3dll ^
-lvulkan-1.lib ^
-lwinmm ^
-lopengl32 ^
-luser32 ^
-lgdi32 ^
-lmsvcrt ^
-nodefaultlibs ^
-o main.exe

@echo Compiled successfully
