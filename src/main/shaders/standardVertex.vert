#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normals;
out vec2 f_texcoord;
//out float brightness;
uniform mat4 model;
uniform mat4 VP;
uniform vec3 directionalLight;
uniform float rollOff;

out vec3 Color;

const vec3 ka = 0.3*vec3(.5, 0.5, 0.5);
const vec3 kd = 0.7*vec3(.5, 0.5, 0.5);

void main() {
  vec3 lightPosition = directionalLight;

  vec4 normal = normalize(model * vec4(normals, 0.0));
  const vec3 LightIntensity = vec3(20);

  float distance = length(lightPosition - vertexPosition_modelspace);
  float intensity = dot(normal, normalize(vec4(lightPosition, 0.0) - vec4(vertexPosition_modelspace, 1.0)));
  gl_Position = VP * model * vec4(vertexPosition_modelspace, 1);

  f_texcoord = texcoord;

  intensity = max(0.0, intensity);
  intensity = (400 * intensity) / (distance * distance * 0.4);

  Color = intensity * LightIntensity * kd + ka;
}
