/**
 * @file GameObjectStructs.hpp
 * @author Christian Galvez
 * @brief Structs used for GameObject classes are defined here. The structs defined here are likely to become
 *          deprecated very soon.
 * @date 2023-07-23
 */
// TODO - Change naming from [...]Structs to [...]Misc

/**
 * @brief Stores info about a GameObject's internal collider object.
 * @param offset(vec4) The distance between the center of the collider and its edges
 *	on the X, Y and Z axis.
 * @param minPoints(vec4) Contains the minimum points used in the automatically
 *	generated box collider for a GameObject.
 * @param center(vec4) The X, Y and Z coordinates of the center of the
 *	collider.
 * @param originalCenter(vec4) The raw X, Y and Z points for the center of the
 *	actual model. These points themselves should not be used for calculating
 *	collision.
 * @param collisionTag(string) The name of the collisionTag associated with the
 *	current GameObject. The collisionTag can be used to determine what two
 *	GameObjects in a scene have collided.
 * @param collider(polygon*) The polygon data for the box collider drawn around a
 *	GameObject it is attached to.
 */
typedef struct colliderInfo {
	vec4 offset, minPoints;
	vec4 center, originalCenter;
	string collisionTag;
	polygon *collider;
} colliderInfo;

/**
 * @brief Used to construct new GameObject instances.
 * @param characterModel(polygon*) Pointer to a polygon struct containing
 *	information about an imported model using the modelImport functions.
 * @param colliderObject(polygon*) Pointer to a polygon struct containing
 *	information about an collider object imported using the modelImport
 *	functions.
 * @param pos(vec3) A 3D vector containing the default position in local space of
 *	the GameObject being created.
 * @param rot(vec3) A 3D vector describing the XYZ rotation to initially apply to
 *	the GameObject upon creation.
 * @param scale(GLfloat) A scalar value that describes the uniform scale to apply
 * to the GameObject upon creation.
 * @param camera(int) Specifies the ID number of the camera in the current
 *	GameInstance to use that the created GameObject will use to get the VP
 *	transformation matrix from (stuff like camera angle and FOV).
 * @param collisionTagName(string) The tag name that the created GameObject will use
 *	when handling collision in the current GameInstance.
*/
typedef struct gameObjectInfo {
	polygon *characterModel;
	vec3 position, rotation;
	GLfloat scale;
	int camera;
	string collisionTagName;
} gameObjectInfo;