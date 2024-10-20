#version 330 core
in vec2 TexCoords;
in float TriDex;
in vec4 tipColor;
out vec4 color;

uniform sampler2D sprite;

void main() {
    int isolatedTriangles[5] = int[](1, 3, 4, 5, 7);
    vec4 texColor = texture(sprite, TexCoords);
		if (texColor.a < 0.1) {
			discard;
		}
    int blank = 0;
    for (int i = 0; i < 5; ++i) {
        if (int(TriDex) == isolatedTriangles[i]) blank = 1;
    }
    if (blank == 1) {
        color = tipColor;
    } else {
        color = texColor;
    }
    
}
