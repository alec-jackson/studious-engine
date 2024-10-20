#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
out float TriDex;

uniform mat4 projection;

void main() {
    int triangle = gl_VertexID / 6;
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
    TriDex = float(triangle);
}
