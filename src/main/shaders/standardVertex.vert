#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normals;
out vec2 f_texcoord;
//out float brightness;
uniform mat4 move;
uniform mat4 scale;
uniform mat4 rotate;
uniform mat4 VP;
uniform vec3 directionalLight;
uniform float rollOff;

out vec3 Color;

const vec3 ka = 0.3*vec3(.5, 0.5, 0.5);
const vec3 kd = 0.7*vec3(.5, 0.5, 0.5);

// Calculates the dot product d * n, finds the angle between the two, sends
// angle ratio to lighting luminance.
// float invRatio(vec3 d, vec3 n) {
//     // Calculate the cross product between the two vectors
//     float i, D, N, preAngle;
//     i = (d[0]*n[0] + d[1]*n[1] + d[2]*n[2]);
//     D = sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
//     N = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
//     return 1 - rollOff * acos(i/(D*N)) / 3.1415926f;

// }

void main() {
  vec3 lightPosition = directionalLight;

  mat4 Model = move*scale*rotate;
  vec4 Normal = normalize(Model * vec4(normals, 0.0));
  const vec3 LightIntensity = vec3(20);

  float distance = length(lightPosition - vertexPosition_modelspace);
  float intensity = dot(Normal, normalize(vec4(lightPosition, 0.0) - vec4(vertexPosition_modelspace, 1.0)));
  gl_Position = VP * move * scale * rotate *  vec4(vertexPosition_modelspace, 1);

  f_texcoord = texcoord;
  //vec4 normal4D = rotate * vec4(normals, 1);
  //vec3 transformedNormals = vec3(1.0f, 1.0f, 1.0f);

  // We want to find the angle from face to directional light.
  // brightness = invRatio(lightPosition, transformedNormals);

  intensity = max(0.0, intensity);
  intensity = (400 * intensity) / (distance * distance * 0.4);

  Color = intensity * LightIntensity * kd + ka;
}
