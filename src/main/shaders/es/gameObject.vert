#version 100

precision mediump float;

attribute vec3 vertexPosition_modelspace;
attribute vec2 texcoord;
attribute vec3 normals;
varying vec2 f_texcoord;
// varying float brightness;
uniform mat4 model;
uniform mat4 VP;
uniform vec3 directionalLight;
uniform float rollOff;

varying vec3 Color;

const vec3 ka = 0.3 * vec3(0.5, 0.5, 0.5);
const vec3 kd = 0.7 * vec3(0.5, 0.5, 0.5);

void main() {
    vec3 lightPosition = directionalLight;

    vec4 normal = normalize(model * vec4(normals, 0.0));
    const vec3 LightIntensity = vec3(20.0);

    float distance = length(lightPosition - vertexPosition_modelspace);
    float intensity = dot(normal, normalize(vec4(lightPosition, 0.0) - vec4(vertexPosition_modelspace, 1.0)));
    gl_Position = VP * model * vec4(vertexPosition_modelspace, 1.0);

    f_texcoord = texcoord;

    intensity = max(0.0, intensity);
    intensity = (400.0 * intensity) / (distance * distance * 0.4);

    Color = intensity * LightIntensity * kd + ka;
}
