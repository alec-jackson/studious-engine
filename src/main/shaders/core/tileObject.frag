#version 330 core
in vec2 TexCoords;
in vec4 vColor;
out vec4 color;

uniform sampler2D sprite;
uniform vec3 tint;

void main() {
    color = vColor;
}
