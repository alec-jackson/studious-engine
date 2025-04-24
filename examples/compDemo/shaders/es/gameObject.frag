#version 100

precision mediump float;

varying vec2 f_texcoord;
// varying float brightness;
uniform sampler2D mytexture;
uniform int hasTexture;
uniform float luminance;

varying vec3 Color;

void main() {
    // Output color = red
    if (hasTexture == 1) {
        vec4 texColor = texture2D(mytexture, f_texcoord);
        if (texColor.a < 0.1) {
            discard;
        }
        texColor.r *= Color.r;
        texColor.g *= Color.g;
        texColor.b *= Color.b;
        // texColor.a *= brightness * luminance;
        gl_FragColor = texColor;
        // gl_FragColor = texture2D(mytexture, f_texcoord);
    } else {
        gl_FragColor = vec4(Color, 1.0);
    }
}
