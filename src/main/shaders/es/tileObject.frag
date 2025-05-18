precision mediump float;

varying vec2 TexCoords;
varying vec4 vColor;
uniform sampler2D sprite;
uniform vec3 tint;

void main() {
    vec4 texColor = texture2D(sprite, TexCoords);
    if (texColor.a < 0.1) {
        discard;
    }
    gl_FragColor = texColor + vec4(tint, 0.0);
}
