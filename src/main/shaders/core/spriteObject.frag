#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;
uniform vec4 tint;

void main() {
    vec4 texColor = texture(sprite, TexCoords);
    if (texColor.a < 0.1) {
        discard;
    }
    color = texColor + vec4(tint);
}
