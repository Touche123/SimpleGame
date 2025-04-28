#ifndef MATERIAL_H
#define MATERIAL_H

#include <cglm/cglm.h>

#include "glad.h"

#define ALBEDO_SLOT 0
#define METALLIC_SLOT 1
#define ROUGHNESS_SLOT 2
#define AO_SLOT 3
#define NORMAL_SLOT 4
#define MATERIAL_TEXTURE_COUNT 5

typedef struct {
    GLuint shaderProgram;

    bool useAlbedoMap;
    bool useMetallicMap;
    bool useRoughnessMap;
    bool useAoMap;
    bool useNormalMap;

    union {
        struct {
            GLuint albedoTexture;
            GLuint metallicTexture;
            GLuint roughnessTexture;
            GLuint aoTexture;
            GLuint normalTexture;
        };
        GLuint textures[MATERIAL_TEXTURE_COUNT];  // 0 = albedo, 1 = metallic, 2 = roughness, 3 = ao, 4 = normal
    };

    vec3 albedoColor;
    float metallicValue;
    float roughnessValue;
    float aoValue;
} Material;

#endif
