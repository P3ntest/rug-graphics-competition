#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal; // normal in view space
uniform sampler2D gAlbedoSpec;
uniform sampler2D gEmission;

// light const
const vec3 lightDir = normalize(vec3(-0.2, -1.0, -0.3));

uniform mat4 projection;
// uniform sampler2D gDepth; // You could sample this as well if needed

vec3 ssrRaycast(vec3 O, vec3 R) {
    float rayLength = 0.0;
    const int maxSteps = 100;
    float stepSize = 0.1;

    for(int i = 0; i < maxSteps; i++) {
        rayLength += stepSize;
        vec3 samplePoint = O + rayLength * R;

        vec4 screenPos = projection * vec4(samplePoint, 1.0);
        screenPos.xyz /= screenPos.w;
        vec2 screenTexCoords = screenPos.xy * 0.5 + 0.5;

        if(screenTexCoords.x < 0.0 || screenTexCoords.x > 1.0 ||
            screenTexCoords.y < 0.0 || screenTexCoords.y > 1.0) {
            return vec3(0.0); // Reflection ray left the screen
        }

        float sceneDepth = -texture(gPosition, screenTexCoords).z;

        const float bias = 1.0;

        float rayDepth = -samplePoint.z;

        if(rayDepth > sceneDepth + bias) {
            // hit!

            vec3 reflectedColor = texture(gEmission, screenTexCoords).rgb;

            // fade todo and potential edge fade

            return reflectedColor;
        }
    }

    return vec3(0.0);
}

void main() {
    // Retrieve data from the G-Buffer using the screen-space texture coordinates
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;   // <-- The Normal Buffer
    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);
    vec3 Albedo = AlbedoSpec.rgb;
    float Reflectiveness = AlbedoSpec.a;

    FragColor = vec4(Normal * 0.5 + 0.5, 1.0); // Map normal from [-1, 1] to [0, 1] range

    // Example access: Visualize the Depth buffer (from gPosition's Z component)
    // float depth = -FragPos.z / 20;
    // FragColor = vec4(depth, depth, depth, 1.0);
    // return;

    // Standard lighting calculation goes here, using FragPos, Normal, and Albedo.

    vec3 diffuseLight = max(dot(normalize(Normal), -lightDir), 0.0) * vec3(0.3);
    vec3 ambientLight = vec3(0.1);

    // compute specular
    vec3 V = normalize(-FragPos); // vector from point to camera
    vec3 L = normalize(-lightDir);
    vec3 R = reflect(-L, normalize(Normal));
    float spec = pow(max(dot(R, V), 0.0), 32);

    vec3 emission = texture(gEmission, TexCoords).rgb;

    vec3 finalColor = (ambientLight + diffuseLight + spec) * Albedo + emission;
    FragColor = vec4(finalColor, 1.0);

    // visualizing the reflectiveness (a of gAlbedoSpec)
    // FragColor = vec4(vec3(Reflectiveness), 1.0);

    if(Reflectiveness > 0.0) {
        vec3 V = normalize(-FragPos); // vector from point to camera
        vec3 R = normalize(reflect(-V, Normal));

        vec3 ssrColor = ssrRaycast(FragPos, R);

        if(length(ssrColor) > 0.01) {
            FragColor = mix(FragColor, vec4(ssrColor, 1.0), Reflectiveness);
        }

    }
}
