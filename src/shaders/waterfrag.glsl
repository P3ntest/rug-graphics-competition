#version 330 core

// Define constants
#define M_PI 3.141593

// Specify the inputs to the fragment shader
// These must have the same type and name!
in vec3 v_worldNormal;
in vec3 v_worldPos;
in vec3 v_screenNormal;

// Specify the Uniforms of the fragment shaders
// uniform vec3 lightPosition; // for example

// Specify the output of the fragment shader
// Usually a vec4 describing a color (Red, Green, Blue, Alpha/Transparency)
out vec4 fColor;

const vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));

void main() {
  vec3 ambient = vec3(0.1, 0.1, 0.3);
  float diff = max(dot(v_worldNormal, lightDir), 0.0);
  vec3 diffuse = diff * vec3(0.0, 0.3, 0.5);
  vec3 viewDir = normalize(-v_worldPos);
  vec3 reflectDir = reflect(-lightDir, v_worldNormal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = spec * vec3(1.0, 1.0, 1.0);
  vec3 color = ambient + diffuse + specular;
  fColor = vec4(color, 1.0);

  // fColor = vec4(v_worldNormal * 0.5 + 0.5, 1.0); // visualize normals
}
