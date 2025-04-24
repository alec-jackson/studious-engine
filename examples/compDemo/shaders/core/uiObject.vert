#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
out float TriDex;
out vec4 tipColor;

uniform mat4 projection;
uniform mat4 model;
uniform float hScale;
uniform float wScale;

vec4 modifiedPos;

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
    switch (vertexIdx) {
        case 0:
            return 0;
        case 1:
        case 4:
            return 1;
        case 2:
        case 3:
            return 3;
        case 5:
            return 2;
        default:
            return -1;
    }
}

int stretchTriangle(int index, float horizontalScale, float verticalScale) {
    int square = index / 6;
    int vertIdx = index % 6;
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
    switch (square) {
        case 1:
        case 4:
        case 7:
            if (corner == 2 || corner == 3) x += horizontalScale;
            break;
        case 2:
        case 5:
        case 8:
            x += horizontalScale;
            break;
        case 0:
        case 3:
        case 6:
        default:
            // Don't do anything here either
            break;
    }

    // Apply scale factor depending on location (vertical)
    switch (square) {
        case 3:
        case 4:
        case 5:
            if (corner == 3 || corner == 0) y += verticalScale;
            break;
        case 0:
        case 1:
        case 2:
            y += verticalScale;
            break;
        case 6:
        case 7:
        case 8:
        default:
            // Don't do anything here either
            break;
    }

    tipColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    modifiedPos = vec4(x, y, 0.0, 1.0);
    
    return square;
}

void main() {
    int triangle = stretchTriangle(gl_VertexID, wScale, hScale);
    gl_Position = projection * model * modifiedPos;
    TexCoords = vertex.zw;
    TriDex = float(triangle);
}
