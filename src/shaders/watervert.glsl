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

uniform float time;

// --- HELPER FUNCTIONS ---

// Dispersion Relation for Shallow Water
// Calculates the angular frequency (omega) based on wavenumber (k) and depth (d).
// omega = sqrt(g * k * tanh(k * d))
float dispersion_omega(float k, float g, float d) {
    // We use a small epsilon to prevent issues with very small k or d
  if(k * d < 0.01) {
        // Shallow water approximation (tanh(x) ~ x)
    return sqrt(g * k * k * d);
  }
  return sqrt(g * k * tanh(k * d));
}

// Calculates the vertical displacement (height) of a single Airy wave component.
// Airy waves are purely vertical displacements, simpler than Gerstner waves.
float getWaveHeightComponent(vec2 xz_pos, float time, float A, float k, vec2 D, float g, float d, float phase_offset) {
    // 1. Calculate Angular Frequency (omega)
  float omega = dispersion_omega(k, g, d);

    // 2. Calculate the Phase
    // Phase = (k * (D . xz_pos)) - (omega * time) + phase_offset
    // D . xz_pos = wave travel distance along its direction
  float phase = k * dot(D, xz_pos) - omega * time + phase_offset;

    // 3. Return the Height
  return A * cos(phase);
}

const float C_GRAVITY = 9.8;    // Gravity (g)
const float C_DEPTH = 5.0; 

// The height function remains the same
float waveHeight(vec2 pos, float time) {
   // Retrieve environment constants
  const float g = C_GRAVITY;
  const float d = C_DEPTH;

  float height = 0.0;

    // --- WAVE COMPONENTS FOR CHOPPY CANAL/RIVER (All Hardcoded) ---

    // Wave 1: Dominant long wave, defining general motion
  const float A1 = 0.15;
  const float k1 = 2.0;
  const vec2 D1 = vec2(1.0, 0.0);

  height += getWaveHeightComponent(pos, time, A1, k1, D1, g, d, 0.0); 

    // Wave 2: Medium wave, crossing direction for chop 
  const float A2 = 0.08;
  const float k2 = 4.5;
  const vec2 D2 = normalize(vec2(0.8, 0.5));

  height += getWaveHeightComponent(pos, time, A2, k2, D2, g, d, 1.2);

    // Wave 3: Small wave, highly choppy
  const float A3 = 0.05;
  const float k3 = 8.0;
  const vec2 D3 = normalize(vec2(0.1, 1.0));

  height += getWaveHeightComponent(pos, time, A3, k3, D3, g, d, 3.5);

    // Wave 4: Tiny, high-frequency ripple 
  const float A4 = 0.02;
  const float k4 = 12.0;
  const vec2 D4 = normalize(vec2(-0.5, -0.7));

  height += getWaveHeightComponent(pos, time, A4, k4, D4, g, d, 5.1);

    // Wave 5: Very high frequency (short wavelength), very small amplitude
  const float A5 = 0.008;
  const float k5 = 18.0;
  const vec2 D5 = normalize(vec2(1.0, -0.2));

  height += getWaveHeightComponent(pos, time, A5, k5, D5, g, d, 6.4); 

    // Wave 6: Micro-ripple, extremely high frequency
  const float A6 = 0.004;
  const float k6 = 25.0;
  const vec2 D6 = normalize(vec2(-0.8, 0.9));

  height += getWaveHeightComponent(pos, time, A6, k6, D6, g, d, 2.7);

  return height * 0.2; // Overall scaling factor
}

// Correctly calculates the normal based on horizontal world coordinates (a vec2)
vec3 calculateWorldNormal(vec2 pos, float time) {
  // for debug just straight up 
  // return vec3(0.0, 1.0, 0.0);

  const float epsilon = 0.0001; // A small offset

  // Tangent in the X direction
  vec3 p1 = vec3(pos.x - epsilon, waveHeight(vec2(pos.x - epsilon, pos.y), time), pos.y);
  vec3 p2 = vec3(pos.x + epsilon, waveHeight(vec2(pos.x + epsilon, pos.y), time), pos.y);
  vec3 tangentX = p2 - p1;

  // Tangent in the Z direction (using pos.y for the Z plane)
  vec3 p3 = vec3(pos.x, waveHeight(vec2(pos.x, pos.y - epsilon), time), pos.y - epsilon);
  vec3 p4 = vec3(pos.x, waveHeight(vec2(pos.x, pos.y + epsilon), time), pos.y + epsilon);
  vec3 tangentZ = p4 - p3;

  // The normal is the cross product of the tangents.
  // Normalize AFTER the cross product.
  return normalize(cross(tangentZ, tangentX));
}

void main() {
  // 1. Get the vertex's original position in world space (without displacement)
  vec4 initialWorldPos = model * vec4(aPos, 1.0);

  // 2. Calculate the world-space normal using the original, flat XZ coordinates
  // This is the key fix: use the non-displaced position to find the slope.
  vec3 worldNormal = calculateWorldNormal(initialWorldPos.xz, time);

  // 3. Now, calculate the final displaced world position
  vec4 finalWorldPos = initialWorldPos;
  finalWorldPos.y += waveHeight(initialWorldPos.xz, time);

  // 4. Transform normal and position for the fragment shader
  // The normal matrix transforms the calculated world normal into the correct orientation
  Normal = normalize(mat3(view) * worldNormal);
  FragPos = vec3(view * finalWorldPos);
  TexCoords = aTexCoords;

  float reflectiveness = 0.45;

  AlbedoReflectance = vec4(vec3(0.0, 0.3, 0.5) * 0.3, reflectiveness); // Water color

  // 5. Finally, transform the displaced vertex to clip space
  gl_Position = projection * view * finalWorldPos;
}