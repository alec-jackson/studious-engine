#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include "common.hpp"
#include "modelImport.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

/*
 colliderInfo contains the scale, translate and rotate matrices for the collider
 for a given object.
*/
typedef struct colliderInfo {
	mat4 scale;
	mat4 translate;
	mat4 rotate;
} colliderInfo;

/*
 gameObjectInfo contains arguments for the createGameObject method inside of the
 GameInstance class. The gameObjectInfo struct contains the following members:
 * (polygon *) characterModel - Reference to a polygon struct containing
 	information about an imported model using the modelImport functions.
 * (polygon *) colliderObject - Reference to a polygon struct containing
 	information about an collider object imported using the modelImport
	functions.
 * (vec3) pos - A 3D vector containing the default position in local space of
 	the GameObject being created.
 * (vec3) rot - A 3D vector describing the XYZ rotation to initially apply to
 	the GameObject upon creation.
 * (GLfloat) scale - A scalar value that describes the uniform scale to apply
   to the GameObject upon creation.
 * (int) camera - Specifies the ID number of the camera in the current
 	GameInstance to use that the created GameObject will use to get the VP
	transformation matrix from (stuff like camera angle and FOV).
 * (string) collisionTagName - The tag name that the created GameObject will use
 	when handling collision in the current GameInstance.
*/
typedef struct gameObjectInfo {
	polygon *characterModel, *colliderObject;
	vec3 pos, rot;
	GLfloat scale;
	int camera;
	string collisionTagName;
} gameObjectInfo;

typedef struct textObjectInfo {
	string message, fontPath;
	GLuint programID;
} textObjectInfo;

/*
 Each GameObject class contains information about a specific object present in
 the current GameInstance. Information present in this class includes current
 world space position, current XYZ rotation, current scale, imported model data,
 directional light, etc. Instance variables in GameObject instances should only
 be modified through public GameObject get/set functions.
*/
class GameObject {
public:
	void configureGameObject(gameObjectInfo objectInfo);
	void setRotation(vec3 rotation);
	void setOrtho(bool ortho);
	void setPos(vec3 pos);
	void setScale(GLfloat uniformScale);
	void setDirectionalLight(vec3 newLight);
	void setVPMatrix(mat4 VPMatrix);
	void setLuminance(GLfloat luminanceValue);
	void setProgramID(GLuint shaderID);
	void setCollider(string coll);
	void drawShape();
	void deleteTextures();
	int getCameraID();
	int getCollision(GameObject *object1, GameObject *object2);
	GLfloat getScale();
	vec3 getPos(vec3 offset);
	vec3 getPos();
	GLuint getProgramID();
	polygon *getModel();
	string getCollider(void);
	mat4 getVPMatrix();
	bool isOrtho();
	int lockObject();
	int unlockObject();

private:
	polygon *model;
	mat4 translateMatrix, scaleMatrix, rotateMatrix; // Model Matrix
	mat4 vpMatrix; // Projection  *View matrix
	GLuint rotateID, scaleID, translateID, vpID, textureID, textCoordsID,
		hasTextureID, directionalLightID, luminanceID, rollOffID, programID,
		MVPID;
	GLint textureCoordID, uniform_mytexture;
	vector<GLint> hasTexture;
	vec3 pos, rot, vel; // Position, rotation and velocity 3D vectors
	GLfloat scale;
	bool configured, orthographic;
	string collisionTag;
	int currentCamera;
	vec3 directionalLight;
	GLfloat luminance, rollOff;
	mutex infoLock;
	polygon *collider;
	unsigned int VAO;
};

typedef struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    ivec2 Size;       // Size of glyph
    ivec2 Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
} Character;

class GameObjectText : public GameObject {
public:
	int initializeText(textObjectInfo info);
	void cleanup();
	void drawText();
	void setMessage(string message);
	string getMessage();

private:
	string message;
	int fontSize;
	unsigned int VAO, VBO;
	map<GLchar, Character> characters;
	unsigned int textureUniformID;

};

/*
 The cameraInfo struct is used as an argument for the createCamera method inside
 of the GameInstance class. Struct members are used in the following manner:
 * (GameObject *) objTarget - Object for camera to target onto.
 * (vec3) offset - 3D vector describing position offset of Camera relative to
 	the target GameObject.
 * (GLfloat) viewCameraAngle - Describes the field of view of the GameCamera to
 	be created.
 * (GLfloat) viewAspectRatio - Describes the aspect ratio of the camera view.
 	This value SHOULD always be set to the current aspect ratio of the SDL
	window, or else things will look stretched out or compressed.
 * (GLfloat) viewNearClipping - Describes the distance for near clipping to
 	occur. This value should be as low as the graphics card allows.
 * (GLfloat) viewFarClipping - Describes the distance for far clipping to occur.
 	This value should be as high as the graphics card allows.
*/
typedef struct cameraInfo {
	GameObject *objTarget;
	vec3 offset;
	GLfloat viewCameraAngle, viewAspectRatio,
	viewNearClipping, viewFarClipping;
} cameraInfo;

/*
 The GameCamera subclass of GameObject is used for information regarding
 active cameras in the GameInstance. This class has more members than the
 normal GameObject class that are specific to properties of the camera.
*/
class GameCamera : public GameObject {
public:
	void configureCamera(cameraInfo camInfo);
	void updateCamera();
	void setOffset(vec3 newOffset);
	void setTarget(GameObject *targetObject);
	void setAspectRatio(GLfloat ratio);
	vec3 getOffset();
	mat4 getVPMatrix();
	GameObject *getTarget();

private:
	GameObject *target;
	vec3 offset;
	GLfloat cameraAngle;
	vec3 currentPosition;
	mat4 VPmatrix;
	GLfloat aspectRatio, nearClipping, farClipping;
};

#endif
