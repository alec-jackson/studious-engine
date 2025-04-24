#version 100

precision mediump float;

varying vec2 TexCoords; // varying is in/out
//varying vec4 color; // out

uniform sampler2D text;
uniform vec3 textColor;
uniform vec3 cutoff;

void main() {
    // Check if we're above the y-cutoff point
    if (gl_FragCoord.y > cutoff.y) {
        discard;
    }
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture2D(text, TexCoords).r);
    gl_FragColor = vec4(textColor, 1.0) * sampled;
}
