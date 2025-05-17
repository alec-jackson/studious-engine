#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in float layer;
layout (location = 2) in mat4 model;
out vec2 TexCoords;
out vec4 vColor;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
    switch (gl_VertexID) {
        case 0:
        case 5:
            vColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 1:
        case 4:
            vColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 2:
        case 3:
            vColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
            break;
        default:
            vColor = vec4(0.2f, 0.5f, 1.0f, 1.0f);
            break;
    }
}
