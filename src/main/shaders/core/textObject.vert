#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 model;
uniform vec3 resolution;

void main() {
    float refW = 1280.0;
    float refH = 720.0;
    float wScale = resolution.x / refW;
    float hScale = resolution.y / refH;
    vec4 pos = projection * model * vec4(vertex.x, vertex.y, 0.0, 1.0);
    // Scale
    pos.x *= wScale;
    pos.y *= hScale;
    // Update translation delta
    pos.x += (((wScale - 1) * refW)) / 2560.0;
    pos.y += (((hScale - 1) * refW)) / 1440.0;
    gl_Position = pos;
    TexCoords = vertex.zw;
}
