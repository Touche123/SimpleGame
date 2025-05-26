#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

#define MAX_LIGHTS 16

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

uniform Light lights[MAX_LIGHTS];
uniform int lightCount;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D normalMap;

uniform bool useAlbedoMap;
uniform bool useMetallicMap;
uniform bool useRoughnessMap;
uniform bool useAoMap;
uniform bool useNormalMap;

uniform vec3 albedoColor;
uniform float metallicValue;
uniform float roughnessValue;
uniform float aoValue;

uniform vec3 camPos;
uniform float exposure;

uniform samplerCube prefilteredMap;     // Prefiltered environment map (for specular reflections)
uniform sampler2D   brdfLUT;            // BRDF lookup texture
uniform samplerCube irradianceMap;      // Diffuse environment map (irradiance)

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometrySchlickGGX(NdotV, roughness);
    float ggx2 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 tonemapUncharted2(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main() {
    vec3 albedo     = useAlbedoMap     ? texture(albedoMap, TexCoord).rgb     : albedoColor;
    float metallic  = useMetallicMap   ? texture(metallicMap, TexCoord).r     : metallicValue;
    float roughness = useRoughnessMap  ? texture(roughnessMap, TexCoord).r    : roughnessValue;
    float ao        = useAoMap         ? texture(aoMap, TexCoord).r           : aoValue;

    vec3 N = normalize(Normal);
    if (useNormalMap) {
        vec3 tangentNormal = texture(normalMap, TexCoord).rgb;
        tangentNormal = tangentNormal * 2.0 - 1.0;
        N = normalize(TBN * tangentNormal);
    }

    vec3 V = normalize(camPos - FragPos);

    // Ackumulera ljusbidrag från alla lampor
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < lightCount; ++i) {
        vec3 L = normalize(lights[i].position - FragPos);
        vec3 H = normalize(V + L);

        float distance    = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lights[i].color * lights[i].intensity * attenuation;

        float NDF         = distributionGGX(N, H, roughness);
        float G           = geometrySmith(N, V, L, roughness);
        vec3 F0           = vec3(0.04);
        F0               = mix(F0, albedo, metallic);
        vec3 F            = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
        vec3 specular     = nominator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD      *= 1.0 - metallic;

        float NdotL = max(dot(N, L), 0.0);
        Lo       += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;

    color = tonemapUncharted2(color * exposure);
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}