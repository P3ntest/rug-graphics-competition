#version 330 core

// ------------------------------------------------------------------
// INPUTS (Interpolated from the G-Buffer Vertex Shader)
// ------------------------------------------------------------------
in vec3 FragPos;      // View-space position
in vec3 Normal;       // View-space normal (not yet normalized)
in vec2 TexCoords;    // Texture coordinates
in vec4 AlbedoReflectance;        // Vertex color

// ------------------------------------------------------------------
// OUTPUTS (Mapped to G-Buffer FBO Color Attachments)
// ------------------------------------------------------------------
layout(location = 0) out vec3 gPosition;   // Renders to gPosition texture (Attachment 0)
layout(location = 1) out vec3 gNormal;     // Renders to gNormal texture (Attachment 1)
layout(location = 2) out vec4 gAlbedoSpec; // Renders to gAlbedoSpec texture (Attachment 2)
layout(location = 3) out vec3 gEmission;   // Renders to gEmission texture (Attachment 3)

// ------------------------------------------------------------------
// UNIFORMS (Material Data)
// ------------------------------------------------------------------

uniform sampler2D texDiffuse;
uniform bool hasDiffuseTex;
uniform sampler2D texEmission;
uniform bool hasEmissionTex;

void main() {
    // 1. Store View-Space Position
    // This provides the depth (Z component) and position for the Lighting Pass.
    gPosition = FragPos;

    // 2. Store View-Space Normal
    // Normalize the normal to ensure correct vector length for lighting calculations.
    gNormal = normalize(Normal);

    gEmission = vec3(1.0, 0.0, 0.0); // red emission for testing

    vec3 Color = AlbedoReflectance.rgb;

    // 3. Store Albedo (Color) and Specular (Shininess/Intensity)
    vec4 albedoColor = vec4(Color, 1.0);
    if(hasDiffuseTex) {
        albedoColor = texture(texDiffuse, TexCoords);
    }

    if(hasEmissionTex) {
        gEmission = texture(texEmission, TexCoords).rgb;
    } else {
        gEmission = vec3(0.0);
    }

    gAlbedoSpec = vec4(albedoColor.rgb, AlbedoReflectance.a);

    // Note: If you have a dedicated Specular Map, you would sample it here 
    // and use the sampled value instead of a fixed uniform.
}