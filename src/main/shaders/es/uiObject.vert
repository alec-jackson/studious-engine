#version 100

precision mediump float;

attribute vec4 vertex; // <vec2 pos, vec2 tex>
attribute float vertexIndex;
varying vec2 TexCoords;
varying float TriDex;
varying vec4 tipColor;

uniform mat4 projection;
uniform mat4 model;
uniform float hScale;
uniform float wScale;

vec4 modifiedPos;

int mod(int x, int y) {
    return x - y * int(float(x) / float(y));
}

int getCorner(int vertexIdx) {
    // This is going to be nasty, so this should be cleaned at some point...
    // Triangle layout is arbitrary, so these values are essentially just mapped
    /* Triangle topology

    Triangle 1 has corners 0, 1, 3
    0---3
    |  /
    | /
    |/
    1

    Triangle 2 has corners 1, 2, 3
        3
       /|
      / |
     /  |
    1---2

    These map to the indexes:
    0---2     3
    |  /     /|
    | /     / |
    |/     /  |
    1     4---5
    */
    if (vertexIdx == 0) return 0;
    if (vertexIdx == 1 || vertexIdx == 4) return 1;
    if (vertexIdx == 2 || vertexIdx == 3) return 3;
    if (vertexIdx == 5) return 2;
    return -1;
}

int stretchTriangle(int index, float horizontalScale, float verticalScale) {
    int square = index / 6;
    int vertIdx = mod(index, 6);
    int corner = getCorner(vertIdx);
    /* Triangle corner topology
    0-----3
    |     |
    |     |
    1-----2
    */

    /* Square topology
    *---*---*---*
    | 0 | 1 | 2 |
    *---*---*---*
    | 3 | 4 | 5 |
    *---*---*---*
    | 6 | 7 | 8 |
    *---*---*---*
    */

    // Apply scale factor depending on location (horizontal)
    float x = vertex.x;
    float y = vertex.y;
    if (square == 1 || square == 4 || square == 7) {
        if (corner == 2 || corner == 3) x += horizontalScale;
    } else if (square == 2 || square == 5 || square == 8) {
        x += horizontalScale;
    } else {
        // Don't do anything here
    }

    // Apply scale factor depending on location (vertical)
    if (square == 3 || square == 4 || square == 5) {
        if (corner == 3 || corner == 0) y += verticalScale;
    } else if (square == 0 || square == 1 || square == 2) {
        y += verticalScale;
    } else {
        // Don't do anything here
    }

    tipColor = vec4(1.0, 1.0, 1.0, 1.0);
    modifiedPos = vec4(x, y, 0.0, 1.0);
    
    return square;
}

void main() {
    int triangle = stretchTriangle(int(vertexIndex), wScale, hScale);
    gl_Position = projection * model * modifiedPos;
    TexCoords = vertex.zw;
    TriDex = float(triangle);
}
