#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

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
	const char *collisionTagName;
	GLuint programIDNo;
	bool orthographic;
} gameObjectInfo;

class GameObject{
public:
  void configureGameObject(gameObjectInfo objectInfo);

  const char *getCollider(void);
  void rotateObject(GLfloat **rotation);
  void sendPosition(GLfloat **pos);
  void sendScale(GLfloat *scale);
  void setDirectionalLight(vec3 newLight);
  void drawShape();
  polygon *getModel();
  void deleteTextures();
  void setVPMatrix(mat4 VPMatrix);
  int getCameraID();
  vec3 getPos(GLfloat *offset);
  vec3 getPos();
  void setLuminance(GLfloat luminanceValue);
  void setLock(pthread_mutex_t *lock);
  int getCollision(GameObject *object1, GameObject *object2);

private:
	polygon *model;
	mat4 translateMatrix, scaleMatrix, rotateMatrix; // Model Matrix
	mat4 vpMatrix; // Projection  *View matrix
	GLuint rotateID, scaleID, translateID, vpID, textureID, textCoordsID, hasTextureID, directionalLightID, luminanceID, rollOffID, programID, MVPID;
	GLint textureCoordID, uniform_mytexture, *hasTexture;
	GLfloat *posX, *posY, *posZ, tPosX, tPosY, tPosZ;
	GLfloat *size;
	GLfloat *rotX, *rotY, *rotZ;
	GLfloat *velX, *velY, *velZ;
	bool dynamicPosition, dynamicRotation, dynamicScaling, configured = false;
	const char *collisionTag;
	int currentCamera;
	vec3 directionalLight;
	GLfloat luminance, rollOff;
	pthread_mutex_t *infoLock;
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
	GLfloat *offset, viewCameraAngle, viewAspectRatio,
	viewNearClipping, viewFarClipping;
} cameraInfo;


class GameCamera{
public:
	void configureCamera(cameraInfo camInfo);
	GameObject *getTarget();
	GLfloat *getOffset();
	mat4 getVPMatrix();
	void updateCamera();
	void setOffset(GLfloat *newOffset);
	void setTarget(GameObject *targetObject);

private:
	GameObject *target;
	GLfloat *offset, cameraAngle;
	GLfloat currentPosition[3];
	mat4 VPmatrix;
	GLfloat aspectRatio, nearClipping, farClipping;
};
