#version 100

precision mediump float;

attribute vec4 vertex; // <vec2 pos, vec2 tex> // Attribute is in
varying vec2 TexCoords; // pass to fragment, must be defined in both

uniform mat4 projection;
uniform mat4 model;

void main() {
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}