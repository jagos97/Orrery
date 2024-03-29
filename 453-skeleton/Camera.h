#pragma once

//------------------------------------------------------------------------------
// This file contains an implementation of a spherical camera
//------------------------------------------------------------------------------

#include <GL/glew.h>
#include <glm/glm.hpp>

class Camera {
public:

	Camera(float t, float p, float r);

	glm::mat4 getView(glm::vec3 pos);
	glm::vec3 getPos();
	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);
	void ChangeAt(glm::vec3 at1);
	void ChangeEye(glm::vec3 at1);

private:

	float theta;
	float phi;
	float radius;
	glm::vec3 at;
	glm::vec3 eye;
};
