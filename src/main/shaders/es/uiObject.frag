#version 100

precision mediump float;

varying vec2 TexCoords;
varying float TriDex;
varying vec4 tipColor;
uniform sampler2D sprite;

void main() {
    // int isolatedTriangles[5] = int[](1, 3, 4, 5, 7);
    vec4 texColor = texture2D(sprite, TexCoords);
    if (texColor.a < 0.1) {
        discard;
    }
    /*
    int blank = 0;
    for (int i = 0; i < 5; ++i) {
        if (int(TriDex) == isolatedTriangles[i]) blank = 1;
    }
    if (blank == 1) {
        gl_FragColor = tipColor;
    } else {
        gl_FragColor = texColor;
    }
    */
    gl_FragColor = texColor;
}
