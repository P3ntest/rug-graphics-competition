#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 AlbedoReflectance;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculate world-space or view-space position
    FragPos = vec3(view * model * vec4(aPos, 1.0));
    // Calculate view-space normal
    Normal = mat3(transpose(inverse(view * model))) * aNormal;

    TexCoords = aTexCoords;

    AlbedoReflectance = vec4(aColor, 0.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}