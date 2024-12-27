#version 100

precision mediump float;

attribute vec4 vertex; // <vec2 pos, vec2 tex>
varying vec2 TexCoords;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 resolution;

void main() {
    // Scale vertex points with resolution
    float vx = resolution.x * vertex.x;
    float vy = resolution.y * vertex.y;
    gl_Position = projection * model * vec4(vx, vy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
