#pragma once
#include "common.hpp"
#include "modelImport.hpp"

using namespace glm;

typedef struct colliderInfo {
	mat4 scale;
	mat4 translate;
	mat4 rotate;
} colliderInfo;

/*
The gameObjectInfo struct is used as an argument for the createGameObject
method. All of the current arguments for createGameObject will be morphed into
this single struct with the same names.
*/
typedef struct gameObjectInfo {
	polygon *characterModel, *colliderObject;
	vec3 scaleVec, pos, rotateAxis;
	GLfloat rotAngle;
	int camera;
	string collisionTagName;
} gameObjectInfo;

class GameObject{
public:
  void configureGameObject(gameObjectInfo objectInfo);

  string getCollider(void);
  void rotateObject(vec3 rotation);
  void sendPosition(vec3 pos);
  void sendScale(GLfloat uniformScale);
  GLfloat getScale();
  void setDirectionalLight(vec3 newLight);
  void drawShape();
  polygon *getModel();
  void deleteTextures();
  void setVPMatrix(mat4 VPMatrix);
  int getCameraID();
  vec3 getPos(vec3 offset);
  vec3 getPos();
  void setLuminance(GLfloat luminanceValue);
  mutex *getLock();
  int getCollision(GameObject *object1, GameObject *object2);

private:
	polygon *model;
	mat4 translateMatrix, scaleMatrix, rotateMatrix; // Model Matrix
	mat4 vpMatrix; // Projection  *View matrix
	GLuint rotateID, scaleID, translateID, vpID, textureID, textCoordsID, hasTextureID, directionalLightID, luminanceID, rollOffID, programID, MVPID;
	GLint textureCoordID, uniform_mytexture;
	vector<GLint> hasTexture;
	vec3 pos, rot, vel; // Position, rotation and velocity 3D vectors
	GLfloat scale;
	bool configured;
	string collisionTag;
	int currentCamera;
	vec3 directionalLight;
	GLfloat luminance, rollOff;
	mutex infoLock;
	polygon *collider;
};

class GameObjectText : GameObject {

};

/*
The cameraInfo struct is used as an argument for the createCamera method. All
of the current arguments for createCamera will be morphed into this single
struct with the same names.
*/
typedef struct cameraInfo {
	GameObject *objTarget;
	vec3 offset;
	GLfloat viewCameraAngle, viewAspectRatio,
	viewNearClipping, viewFarClipping;
} cameraInfo;


class GameCamera{
public:
	void configureCamera(cameraInfo camInfo);
	GameObject *getTarget();
	vec3 getOffset();
	mat4 getVPMatrix();
	void updateCamera();
	void setOffset(vec3 newOffset);
	void setTarget(GameObject *targetObject);

private:
	GameObject *target;
	vec3 offset;
	GLfloat cameraAngle;
	vec3 currentPosition;
	mat4 VPmatrix;
	GLfloat aspectRatio, nearClipping, farClipping;
};
