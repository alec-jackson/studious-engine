#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform vec3 cutoff;

void main() {
    // Check if we're above the y-cutoff point
    if (gl_FragCoord.y > cutoff.y) {
        discard;
    }
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
