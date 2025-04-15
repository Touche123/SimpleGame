#version 330 core
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D texture_diffuse;
uniform vec3 lightDir;  // riktning mot ljuskällan (normaliserad)
uniform vec3 lightColor;
uniform vec3 objectColor;


void main() {
    float diff = max(dot(normalize(Normal), -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 texColor = texture(texture_diffuse, TexCoord).rgb;
    //vec3 result = (diffuse + 0.1) * texColor * objectColor;  // 0.1 = ambient ljus
    vec3 result = (diffuse + 0.1) * objectColor;  // 0.1 = ambient ljus

    FragColor = vec4(result, 1.0);
}
