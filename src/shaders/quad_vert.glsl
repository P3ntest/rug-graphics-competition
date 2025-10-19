#version 330 core
layout(location = 0) in vec2 aPos; // Input for the quad's NDC position
layout(location = 1) in vec2 aTexCoords; // Input for the quad's texture coordinates

out vec2 TexCoords;

void main() {
    // Pass texture coordinates directly to the fragment shader
    TexCoords = aTexCoords;

    // The position is already in clip space (NDC), so pass it directly
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}