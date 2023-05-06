#include "collider.hpp"

Polygon Collider::buildBox(const Polygon &target, GLuint programID) {
    GLfloat min[3] = {999, 999, 999}, tempMin[3] = {999, 999, 999}; // TODO - unscuff this
    GLfloat max[3] = {-999, -999, -999}, tempMax[3] = {-999, -999, -999};
    glfVVec::iterator it;
    // Go through objects and get absolute min/max points
    for (it = target.getVertices().begin(); it != target.getVertices().end(); ++it) {
        for (int i = 0; i < 3; i++) {
            tempMin[i] = getVert((*it), i, min_func);
            tempMax[i] = getVert((*it), i, max_func);
            if (tempMin[i] < min[i]) {
                min[i] = tempMin[i];
            }
            if (tempMax[i] > max[i]) {
                max[i] = tempMax[i];
            }
        }
    }
    // Create new polygon object for collider
    // Manually build triangles for cube collider
    vector<GLfloat> colliderVertices = {
        // First face
        min[0], min[1], min[2],
        min[0], min[1], max[2],
        min[0], max[1], min[2],
        min[0], max[1], max[2],
        min[0], max[1], min[2],
        min[0], min[1], max[2],
        // Second face
        min[0], min[1], max[2],
        min[0], max[1], max[2],
        max[0], min[1], max[2],
        min[0], max[1], max[2],
        max[0], min[1], max[2],
        max[0], max[1], max[2],
        // Third face
        max[0], max[1], max[2],
        max[0], min[1], max[2],
        max[0], max[1], min[2],
        max[0], min[1], min[2],
        max[0], min[1], max[2],
        max[0], max[1], min[2],
        // Fourth face
        max[0], max[1], min[2],
        min[0], max[1], min[2],
        max[0], min[1], min[2],
        min[0], min[1], min[2],
        min[0], max[1], min[2],
        max[0], min[1], min[2],
        // Fifth face
        min[0], max[1], min[2],
        max[0], max[1], min[2],
        min[0], max[1], max[2],
        max[0], max[1], max[2],
        max[0], max[1], min[2],
        min[0], max[1], max[2],
        // Sixth Face
        min[0], min[1], min[2],
        max[0], min[1], min[2],
        min[0], min[1], max[2],
        max[0], min[1], max[2],
        max[0], min[1], min[2],
        min[0], min[1], max[2]
    };
    
    auto collider = Polygon(
        0, //shapebufferId
        UINT_MAX, // textureCoordsId
        UINT_MAX, // textureId
        colliderVertices, // vertices
        108, // pointCount
        programID
    );

    glGenBuffers(1, collider.getShapeBufferIdAddr(0));
    glBindBuffer(GL_ARRAY_BUFFER, collider.getShapeBufferId(0));
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 108,
        (void *) collider.getVerticiesLocation(0, 0), GL_STATIC_DRAW);
    
    // Set the correct center points
    for (int i = 0; i < 3; i++) {
        center[i] = max[i] - ((abs(max[i] - min[i])) / 2);
    }
    center[3] = 1; // SET W!!!
    originalCenter = center;
    // Update the offset for the collider to be distance between center and edge
    for (int i = 0; i < 3; i++) {
        minPoints[i] = min[i];
    }
    minPoints[3] = 1; // SET W!!!
    this->programID = programID;
    return collider;
}

Collider::Collider(string tag, GLuint programID, const Polygon &target) :
    collider(buildBox(target, programID)) {
    cout << "Building collider for " << tag << endl;
}

/*
 (GLfloat) getVert takes some (vector<GLfloat>) vertices, takes an (int) axis
 defining the axis we're looking at specifically (X, Y, Z -> 0, 1, 2
 respectively). The test for the returned vertex is defined in the passed in
 (bool (*test)(float a, float b)) function. An example parameter function can be
 seen in the built in max_func and min_func functions.

 (GLfloat) returns the desired vertex point on success. On error, 0.0f is
 returned and an error message is printed to stderr.
*/
GLfloat Collider::getVert(vector<GLfloat> vertices, int axis,
    bool (*test)(float a, float b)) {
    if (vertices.size() < 3) {
        cerr << "Error: Vertices vector is empty!\n";
        return 0.0f;
    }
    GLfloat currentMin = vertices[axis];
    for (int i = 0; i < vertices.size() / 3; i++) {
        GLfloat tempMin = vertices[i * 3 + axis];
        if (test(tempMin, currentMin)) {
            currentMin = tempMin;
        }
    }
    return currentMin;
}

GLuint Collider::getProgramId() {
    return programID;
}

vec4 Collider::getMinPoints() {
    return minPoints;
}

vec4 Collider::getOriginalCenter() {
    return originalCenter;
}

vec4 Collider::getCenter() {
    return center;
}

GLfloat Collider::getOffset(unsigned int index) {
    if (index > 4) throw runtime_error("Index out of bounds!");
    return offset[index];
}

GLfloat Collider::getCenter(unsigned int index) {
    if (index > 4) throw runtime_error("Index out of bounds!");
    return center[index];
}

const Polygon &Collider::getPolygon() const {
    return collider;
}

void Collider::setOffset(unsigned int index, GLfloat val) {
    if (index > 4) throw runtime_error("Index out of bounds!");
    offset[index] = val;
}

void Collider::setCenter(vec4 newCenter) {
    this->center = newCenter;
}

void Collider::setTag(string collisionTag) {
    this->collisionTag = collisionTag;
}
