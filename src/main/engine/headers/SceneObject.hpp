/**
 * @author Christian Galvez
 * @date 2023-07-23
 * @brief This is used as the base class for all objects used in GameObject scenes (GameObject, TextObject,
 *        CameraObject, etc). This base class should only contain methods and member variables that are common across
 *        these objects. This class is not designed to be used directly for anything.
 * 
 */
class SceneObject {
    public:
        // Setter methods
        inline void setVpMatrix(mat4 vpMatrix) { this->vpMatrix = vpMatrix; };
        inline void setPosition(vec3 position) { this->position = position; };
        inline void setRotation(vec3 rotation) { this->rotation = rotation; }; 

        // Getter methods
        inline mat4 getVpMatrix() { return this->vpMatrix; };
        inline vec3 getPosition() { return this->position; };
        inline vec3 getPosition(vec3 offset) { return this->position + offset; };
        inline vec3 getRotation() { return this->rotation; };

    protected:
        mat4 translateMatrix;
        mat4 scaleMatrix;
        mat4 rotateMatrix;
        mat4 vpMatrix;

        vec3 position;
        vec3 rotation;
        vec3 velocity;
};
