#version 310 es
precision mediump float;
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in float layer;
layout (location = 2) in mat4 model;
out vec3 TexCoords;

uniform mat4 projection;

void main() {
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vec3(vertex.z, vertex.w, layer);
}
