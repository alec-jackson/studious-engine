#include "polygon.hpp"

/*
 (bool) min_func takes some (float) a and (float) b values and returns the
 boolean (a < b), so true if a is less than b, otherwise false.
*/
bool min_func(float a, float b) {
    return a < b;
}

/*
 (bool) max_func takes some (float) a and (float) b values and returns the
 boolean (a > b), so true if a is greater than b, otherwise false.
*/
bool max_func(float a, float b) {
    return a > b;
}

class Collider { 
private:
    vec4 offset, minPoints;
    vec4 center, originalCenter;
    string collisionTag;
    Polygon collider;
    GLuint programID;
public:
    Collider(string tag, GLuint programID, const Polygon &target);
    GLfloat getVert(vector<GLfloat> vertices, int axis,
        bool (*test)(float a, float b));
    Polygon buildBox(const Polygon &target, GLuint programID);
    GLuint getProgramId();
    vec4 getMinPoints();
    vec4 getOriginalCenter();
    vec4 getCenter();
    GLfloat getOffset(unsigned int index);
    GLfloat getCenter(unsigned int index);
    const Polygon &getPolygon() const;
    void setOffset(unsigned int index, GLfloat val);
    void setCenter(vec4);
    void setTag(string collisionTag);
    inline Collider() {};
};
