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

// Perlin Noise from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 c) {
  return fract(sin(dot(c.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

const float PI = 3.14159265359;

float noise(vec2 p, float freq) {
  float unit = freq;
  vec2 ij = floor(p / unit);
  vec2 xy = mod(p, unit) / unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
  xy = .5 * (1. - cos(PI * xy));
  float a = rand((ij + vec2(0., 0.)));
  float b = rand((ij + vec2(1., 0.)));
  float c = rand((ij + vec2(0., 1.)));
  float d = rand((ij + vec2(1., 1.)));
  float x1 = mix(a, b, xy.x);
  float x2 = mix(c, d, xy.x);
  return mix(x1, x2, xy.y);
}

float pNoise(vec2 p, int res) {
  float persistance = .5;
  float n = 0.;
  float normK = 0.;
  float f = 4.;
  float amp = 1.;
  int iCount = 0;
  for(int i = 0; i < 50; i++) {
    n += amp * noise(p, f);
    f *= 2.;
    normK += amp;
    amp *= persistance;
    if(iCount == res)
      break;
    iCount++;
  }
  float nf = n / normK;
  return nf * nf * nf * nf;
}

// END Perlin Noise

float sin_octaves(float x, int octaves) {
  float total = 0.0;
  float frequency = 1.0;
  float amplitude = 1.0;
  float maxValue = 0.0; // Used for normalizing result to [0,1]

  for(int i = 0; i < octaves; i++) {
    total += sin(x * frequency) * amplitude;

    maxValue += amplitude;

    amplitude *= 0.5;
    frequency *= 2.0;
  }

  return total / maxValue;
}

// The height function remains the same
float waveHeight(vec2 pos, float time) {
  // return 0;
  return sin_octaves(pos.x * 2.0 + time + pos.y * 3, 4) * 0.08 +
    // sin_octaves(pos.x * 1.5 - time * 0.5 + pos.y * 2, 3) * 0.05 +
    // sin_octaves(pos.x * 0.5 + time * 0.2 + pos.y * 1, 2) * 0.02 +
    // add some high frequency detail
    pNoise(pos * 5.0 + vec2(time) * 1.0, 4) * 0.1;
}

// Correctly calculates the normal based on horizontal world coordinates (a vec2)
vec3 calculateWorldNormal(vec2 pos, float time) {
  // for debug just straight up 
  // return vec3(0.0, 1.0, 0.0);

  const float epsilon = 0.001; // A small offset

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
  Normal = normalize(inverse(transpose(mat3(view * model))) * worldNormal);
  FragPos = vec3(view * finalWorldPos);
  TexCoords = aTexCoords;

  float reflectiveness = 0.6;

  AlbedoReflectance = vec4(vec3(0.0, 0.3, 0.5) * 0.7, reflectiveness); // Water color

  // 5. Finally, transform the displaced vertex to clip space
  gl_Position = projection * view * finalWorldPos;
}