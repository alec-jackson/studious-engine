#version 330 core
in vec3 TexCoords;
in vec4 vColor;
out vec4 color;

uniform sampler2DArray sprite;
uniform vec3 tint;

void main() {
    vec4 texColor = texture(sprite, TexCoords);
		if (texColor.a < 0.1) {
			discard;
		}
    color = texColor + vec4(tint, 0.0);
}
