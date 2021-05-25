#ifndef REQUIRED_LIBS
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <math.h>
#include <cstring>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#define SDL2_image
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;
using namespace std;
#define REQUIRED_LIBS
#endif

#include "modelImport.hpp"
#include "gameObject.hpp"

void GameObject::configureGameObject(gameObjectInfo objectInfo) {
	//Arbitrary directional light value (PREVENT SEGFAULT)
	programID = objectInfo.programIDNo;
	collider = objectInfo.colliderObject;
	luminance = 1.0f;
	rollOff = 0.9f;
	directionalLight = vec3(0,0,0);
	model = objectInfo.characterModel;
	printf("Preparing to configure the game object\n");
	// Create the hasTexture array for each object to have a single entry
	hasTexture = (GLint*)malloc(sizeof(GLint)*model->numberOfObjects);
	configured = true;
	currentCamera = objectInfo.camera;
	printf("Currently populating the hasTexture array!\n");
	//printf("Upper loop is %i\n", model->numberOfObjects);
	// Populate the hasTexture array with texture info
	for (int i = 0; i < model->numberOfObjects; i++) {
		if (model->textureCoords[i] == NULL) {
			hasTexture[i] = 0; // The current object does not have a texture
		} else {
			hasTexture[i] = 1; // The current object does have a texture
		}
	}
	dynamicPosition = false;
	dynamicRotation = false;
	dynamicScaling = false;
	scaleMatrix = glm::scale(objectInfo.scaleVec);
	translateMatrix = glm::translate(mat4(1.0f), objectInfo.pos);
	rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(objectInfo.rotAngle),
	objectInfo.rotateAxis);
	collisionTag = objectInfo.collisionTagName;
	rotateID = glGetUniformLocation(programID, "rotate");
	scaleID = glGetUniformLocation(programID, "scale");
	translateID = glGetUniformLocation(programID, "move");
	vpID = glGetUniformLocation(programID, "VP");
	hasTextureID = glGetUniformLocation(programID, "hasTexture");
	directionalLightID = glGetUniformLocation(programID, "directionalLight");
	luminanceID = glGetUniformLocation(programID, "luminance");
	rollOffID = glGetUniformLocation(programID, "rollOff");
	if (collider != NULL) {
		MVPID = glGetUniformLocation(collider->programID, "MVP");
	}
	vpMatrix = mat4(1.0f); // Set the VP matrix to the identity matrix by default.
	tPosX = objectInfo.pos[0];
	tPosY = objectInfo.pos[1];
	tPosZ = objectInfo.pos[2];
}

const char* GameObject::getCollider(void) {
	if (collisionTag == NULL) {
		fprintf(stderr, "GameObject was not assigned a collider tag!");
		return NULL;
	}
	return collisionTag;
}
// rotateObject takes a vec3 containing x, y and z rotation in degrees.
void GameObject::rotateObject(GLfloat **rotation) {
	rotX = rotation[0];
	rotY = rotation[1];
	rotZ = rotation[2];
	dynamicRotation = true;
}
void GameObject::sendPosition(GLfloat **pos) {
	posX = pos[0];
	posY = pos[1];
	posZ = pos[2];
	dynamicPosition = true;
}
void GameObject::sendScale(GLfloat *scale) {
	size = scale;
	dynamicScaling = true;
}
void GameObject::setDirectionalLight(vec3 newLight) {
	directionalLight = newLight;
}
void GameObject::drawShape() {
	//printf("Starting to draw the shape!\n");
	// Draw each shape individually
	for (int i = 0; i < model->numberOfObjects; i++) {
		//printf("Currently drawing %s[%i]\n", collisionTag, i);
		glUseProgram(programID);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (!configured) {
			fprintf(stderr, "GameObject with tag %s has not been configured yet!\n", collisionTag);
		}
		pthread_mutex_lock(infoLock);
		if (dynamicPosition) {
			translateMatrix = glm::translate(mat4(1.0f), vec3(*posX, *posY, *posZ));
		}
		if (dynamicRotation) {
			rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(*rotX), vec3(1,0,0))  *glm::rotate(mat4(1.0f), glm::radians(*rotY), vec3(0,1,0))  *glm::rotate(mat4(1.0f), glm::radians(*rotZ), vec3(0,0,1));
		}
		if (dynamicScaling) {
			scaleMatrix = glm::scale(vec3(*size, *size, *size));
		}
		glUniform1f(luminanceID, luminance);
		glUniform1f(rollOffID, rollOff);
		glUniform3fv(directionalLightID, 1, &directionalLight[0]);
		glUniformMatrix4fv(vpID, 1, GL_FALSE, &vpMatrix[0][0]);
		//printf("Drawing - DynamicPosition %i \n", dynamicPosition);
		glUniformMatrix4fv(translateID, 1, GL_FALSE, &translateMatrix[0][0]);
		glUniformMatrix4fv(scaleID, 1, GL_FALSE, &scaleMatrix[0][0]);
		glUniformMatrix4fv(rotateID, 1, GL_FALSE, &rotateMatrix[0][0]);
		glUniform1i(hasTextureID, hasTexture[i]);
		//printf("Starting draw\n");
		pthread_mutex_unlock(infoLock);
		if (hasTexture[i]) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model->textureID[i]);
			glUniform1i(model->textureUniformID, 0);
			//printf("Set texture\n");
		}
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, model->shapebufferID[i]);
		//printf("Bound the vertex buffer\n");
		glVertexAttribPointer(
							  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
							  3,                  // size
							  GL_FLOAT,           // type
							  GL_FALSE,           // normalized?
							  0,                  // stride
							  (void*)0            // array buffer offset
							  );
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, model->normalbufferID[i]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//printf("Starting to draw shape");
		if (hasTexture[i]) {
			//printf("Running texture elements for object %s\n", collisionTag);
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, model->textureCoordsID[i]);
			glVertexAttribPointer(
				1,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,
				(void*)0
			);
			glDrawArrays(GL_TRIANGLES, 0, model->pointCount[i]  *3);
			glDisableVertexAttribArray(1);
		} else {
			glDrawArrays(GL_TRIANGLES, 0, model->pointCount[i]  * 3);
		}
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(0);
	}
	if (collider != NULL) {
		pthread_mutex_lock(infoLock);
		// After drawing the gameobject, draw the collider
		glUseProgram(collider->programID); // grab the programID from the object
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		mat4 MVP = vpMatrix * translateMatrix * scaleMatrix * rotateMatrix;
		glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
		pthread_mutex_unlock(infoLock);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, collider->shapebufferID[0]);
		//printf("Bound the vertex buffer\n");
		glVertexAttribPointer(
							  0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
							  3,                  // size
							  GL_FLOAT,           // type
							  GL_FALSE,           // normalized?
							  0,                  // stride
							  (void*)0            // array buffer offset
							  );
		glDrawArrays(GL_TRIANGLES, 0, collider->pointCount[0] * 3);
		glDisableVertexAttribArray(0);
	}
}
// Returns a pointer to the gameObject's polygon data (contains vertices)
// for the shape.
polygon* GameObject::getModel() {
	return model;
}

void GameObject::deleteTextures() {
	for (int i = 0; model->numberOfObjects; i++) {
		if (hasTexture[i]) {
			glDeleteTextures(1, &textureID);
			hasTexture[i] = false;
		}
	}
}
void GameObject::setVPMatrix(mat4 VPMatrix) {
	vpMatrix = VPMatrix;
}
int GameObject::getCameraID() {
	return currentCamera;
}
vec3 GameObject::getPos(GLfloat *offset) {
	if (dynamicPosition) {
		return vec3(*posX + offset[0], *posY + offset[1], *posZ + offset[2]);
	} else {
		return vec3(tPosX + offset[0], tPosY + offset[1], tPosZ + offset[2]);
	}
}
vec3 GameObject::getPos() {
	if (dynamicPosition) {
		return vec3(*posX, *posY, *posZ);
	} else {
		return vec3(tPosX, tPosY, tPosZ);
	}
}
void GameObject::setLuminance(GLfloat luminanceValue) {
	luminance = luminanceValue;
}
void GameObject::setLock(pthread_mutex_t *lock) {
	infoLock = lock;
}

/*
 getCollision takes two GameObjects, (GameObject *)object1 and
 (GameObject *)object2 and checks if they are colliding with one another.
 Function will return -1 if object1 or object2 are missing collider objects.
 Otherwise, function will return 0 when no collision is happening and 1
 if the two objects are colliding.
 */
int GameObject::getCollision(GameObject *object1, GameObject *object2) {
  vec3 center = vec3(0.0f, 0.0f, 0.0f);
  // Check for collision objects on object1 and object2
  if (object1 == NULL || object2 == NULL || object1->collider == NULL
      || object2->collider == NULL) {
    return -1;
  }
  // Assume rectangular box collider
  return 0;
}

/* GameCamera objects should be created with a create camera function inside of
 the GameInstance class.
*/

// Camera will follow around a given target.
void GameCamera::configureCamera(cameraInfo camInfo) {
	aspectRatio = camInfo.viewAspectRatio;
	nearClipping = camInfo.viewNearClipping;
	farClipping = camInfo.viewFarClipping;
	offset = camInfo.offset;
	target = camInfo.objTarget;
	cameraAngle = camInfo.viewCameraAngle;
	return;
}
GameObject* GameCamera::getTarget() {
	return target;
}
GLfloat* GameCamera::getOffset() {
	return offset;
}
mat4 GameCamera::getVPMatrix() {
	return VPmatrix;
}
// Call updateCamera every frame to update VP matrix
void GameCamera::updateCamera() {
	if (target == NULL) {
		printf("UNABLE TO UPDATE CAMERA! NULL TARGET\n");
		return;
	}
	mat4 viewMatrix = lookAt(target->getPos(offset), target->getPos(), vec3(0,1,0));
	mat4 projectionMatrix = perspective(radians(cameraAngle), aspectRatio, nearClipping, farClipping);
	VPmatrix = projectionMatrix * viewMatrix;
}
// If you want to change the offset inside of the game
void GameCamera::setOffset(GLfloat *newOffset) {
	offset = newOffset;
}
void GameCamera::setTarget(GameObject *targetObject) {
	target = targetObject;
}
