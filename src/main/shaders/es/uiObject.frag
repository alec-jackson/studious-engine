#version 310 es
precision mediump float;
in vec2 TexCoords;
in float TriDex;
in vec4 tipColor;
out vec4 color;

uniform sampler2D sprite;

void main() {
    //int isolatedTriangles[5] = int[](1, 3, 4, 5, 7);
    vec4 texColor = texture(sprite, TexCoords);
    if (texColor.a < 0.1) {
        discard;
    }
    color = texColor;
    /*
    int blank = 0;
    for (int i = 0; i < 5; ++i) {
        if (int(TriDex) == isolatedTriangles[i]) blank = 1;
    }
    if (blank == 1) {
        color = tipColor;
    } else {
        color = texColor;
    }
    */
    /* Visual debugging :)
    float magnitude = cornerDex / 3.0f;
    switch (int(TriDex)) {
        case 0:
        case 1:
        case 2:
            color = vec4(magnitude * 1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 3:
        case 4:
        case 5:
            color = vec4(0.0f, magnitude * 1.0f, 0.0f, 1.0f);
            break;
        case 6:
        case 7:
        case 8:
            color = vec4(0.0f, 0.0f, magnitude * 1.0f, 1.0f);
            break;
        default:
            color = texColor;
            break;
    }*/
    
}
