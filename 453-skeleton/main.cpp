#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

const float PI = 3.14159265359;
int size;
unsigned int* p;
bool indexMade = false;
bool animate = true;
float speed = 1;
glm::mat4 I(1.f);

float const halfCircleSize = 25;
float const surfaceSize = 25;
int const pSize = ((2*25)+2) *25 * 6 + 1;
int index = 0;


glm::vec3 red(1.0f, 0.f, 0.f);
glm::vec3 green(.0f, 1.f, 0.f);
glm::vec3 blue(.0f, 0.f, 1.f);
glm::vec3 yellow(1.0f, 1.f, 0.f);
glm::vec3 magenta(1.0f, 0.f, 1.f);
glm::vec3 cyan(.0f, 1.f, 1.f);
glm::vec3 black(0.0f, 0.f, 0.f);
glm::vec3 white(1.f, 1.f, 1.f);


// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	//gpuGeom.setCols(cpuGeom.cols);
	gpuGeom.setNormals(cpuGeom.normals);
	gpuGeom.setTexCoords(cpuGeom.texCoords);
}

void TransformVertices(glm::mat4 transformation, CPU_Geometry& n, std::vector<glm::vec3> o) {
	n.verts.clear();
	n.verts.resize(o.size());
	for (int i = 0; i < o.size(); i++) {
		n.verts.at(i) = glm::vec3(transformation * glm::vec4(o.at(i),1.f));
	}
}

glm::mat4 MakeTranslationMatrixXYZ(float x, float y, float z) {
	glm::mat4 translation(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		x, y, z, 1.f
	);
	return translation;
}

glm::mat4 MakeTranslationMatrixVEC(glm::vec3 a) {
	glm::mat4 translation(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		a.x, a.y, a.z, 1.f
	);
	return translation;
}


glm::mat4 MakeScaleMatrix(float scale) {
	glm::mat4 scaling(
		scale, 0.f, 0.f, 0.f,
		0.f, scale, 0.f, 0.f,
		0.f, 0.f, scale, 0.f,
		0.f, 0.f, 0.f, 1.f
	);
	return scaling;
}



// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 0.0, 6.0), aspect(1.0f) {
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_SPACE) {
				animate = !animate;
			}
			else if (key == GLFW_KEY_D) {
				speed += 0.05;
			}
			else if (key == GLFW_KEY_A) {
				speed -= 0.05;
				if (speed < 0) speed = 0;
			}
			else if (key == GLFW_KEY_R) {
				reset = true;
			}
			else if (key == GLFW_KEY_W) {
				index = 1;
			}
			else if (key == GLFW_KEY_S) {
				index = 0;
			}

		}
		else if (action == GLFW_REPEAT){
			if (key == GLFW_KEY_D) {
				speed += 0.05;
			}
			else if (key == GLFW_KEY_A) {
				speed -= 0.05;
				if (speed < 0) speed = 0;
			}
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp, glm::mat4 t, glm::vec3 pos) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView(pos);
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint shadeLoc = glGetUniformLocation(sp, "shade");
		glUniform1f(shadeLoc, 0);
		shadeLoc = glGetUniformLocation(sp, "camPos");
		glm::vec3 cam = camera.getPos() + pos;
		glUniform3fv(shadeLoc, 1, glm::value_ptr(cam));

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
		uniMat = glGetUniformLocation(sp, "T");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(t));

	}

	Camera camera;
	bool reset = false;;

private:

	bool rightMouseDown = false;
	float aspect;
	double mouseOldX;
	double mouseOldY;

};

struct Object {
	Object(std::string texturePath, GLenum textureInterpolation, float rotate, float revo) :
		texture(texturePath, textureInterpolation),
		transformation(1.0f), // This constructor sets it as the identity matrix
		position(0),
		rotation(rotate),
		revolution(revo),
		defaultTheta(PI)
	{}

	Texture texture;
	glm::vec3 position;
	glm::vec3 defaultpos;
	glm::vec3 relativePositionSun;
	glm::vec3 relativePositionToPlanet;
	glm::vec3 defaultSun;
	glm::vec3 defaultPlanet;
	glm::mat4 transformation;
	glm::mat3 defaultTransform;
	float revolution = 0;
	float rotation = 0;
	float theta = PI;
	float defaultTheta;

};


void MakeHalfCircle(CPU_Geometry &cpu) {
	for (float i = 0; i <= PI+ 0.001f; i += PI / halfCircleSize) {
		cpu.verts.push_back(glm::vec3(sinf(i), cosf(i), 0.f));
	}
	cpu.verts.push_back(glm::vec3(0, -1.f, 0.f));
	cpu.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
}

void PutPointsIntoSurface(CPU_Geometry& surface, std::vector<std::vector<glm::vec3>>& const points) {
	if (!indexMade) {
		static unsigned int temp[pSize];
		int index = 0;
		int last = points.size() - 1;
		int last2 = points.at(0).size() - 1;
		int sizeOfx = points.size();
		int sizeOfy = points.at(0).size();

		for (auto i = points.begin(); i < points.end() - 1; i++) {
			for (int j = 0; j < (*i).size() - 1; j++) {
				temp[index] = (i - points.begin()) * sizeOfy + j + 1;
				temp[index + 1] = (i - points.begin()) * sizeOfy + j;
				temp[index + 2] = (i - points.begin() + 1) * sizeOfy + j + 1;
				temp[index + 3] = (i - points.begin() + 1) * sizeOfy + j + 1;
				temp[index + 4] = (i - points.begin()) * sizeOfy + j;
				temp[index + 5] = (i - points.begin() + 1) * sizeOfy + j;
				index += 6;
			}
		}

		size = index;
		p = temp;
		indexMade = true;
	}

}



void MakeSurface(CPU_Geometry lines, CPU_Geometry& surface) {
	surface.verts.clear();
	float num = lines.verts.size();
	float yt = 1.f;
	if (lines.verts.size() > 0) {
		int m = 0;
		std::vector<std::vector<glm::vec3>> points;
		for (float u = 0; u <= 2 * PI +0.001f; u += PI/surfaceSize) {
			float xt = 1 - u / (2 * PI);
			
			points.push_back(std::vector<glm::vec3>());
			for (auto i = lines.verts.begin(); i < lines.verts.end(); i++) {
				float xp = (*i).x * cosf(u);
				float yp = (*i).y;
				float zp = (*i).x * sinf(u);
				points.at(m).push_back(glm::vec3(xp,yp,zp));
				surface.verts.push_back(glm::vec3(xp,yp,zp));
				surface.normals.push_back(glm::vec3(xp, yp, zp));
				surface.texCoords.push_back(glm::vec2(xt, yt));
				yt -= 1.f / num;
			}
			m++;
			yt = 1.f;
		}
	PutPointsIntoSurface(surface, points);
	}
}



int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	auto sun = std::make_shared<Object>("textures\\sun.jpg", GL_LINEAR, PI/1000, 0);
	auto earth = std::make_shared<Object>("textures\\earth.jpg", GL_LINEAR, PI/ 200, PI/3000);
	auto moon = std::make_shared<Object>("textures\\moon.jpg", GL_LINEAR, PI/1000, PI/1000);
	auto stars = std::make_shared<Object>("textures\\stars.jpg", GL_LINEAR, 0, 0);
	auto night = std::make_shared<Object>("textures\\night.jpg", GL_LINEAR, 0, 0);
	auto clouds = std::make_shared<Object>("textures\\clouds.png", GL_LINEAR, PI/120, 0);
	auto bump = std::make_shared<Object>("textures\\bump.jpg", GL_LINEAR, 0, 0);



	GLDebug::enable();
	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// The current CPU_Geometry and GPU_Geometry classes are defined in
	// Geometry.h/Geometry.cpp They will work for this assignment, but for some of
	// the bonuses you may have to modify them.


	CPU_Geometry halfCircle;
	CPU_Geometry earthSphere;


	MakeHalfCircle(halfCircle);
	MakeSurface(halfCircle, earthSphere);

	glPointSize(10.0f);

	earth->transformation = MakeTranslationMatrixXYZ(0, 0, -4) * glm::rotate(I, PI / 4, glm::vec3(1, 0, 0)) * MakeScaleMatrix(0.6);
	clouds->transformation = MakeTranslationMatrixXYZ(0, 0, -4) * glm::rotate(I, PI / 4, glm::vec3(1, 0, 0)) * MakeScaleMatrix(0.65);
	moon->transformation = MakeTranslationMatrixXYZ(0, 0, -6)  * glm::rotate(I, PI / 4, glm::vec3(1, 0, 0)) * MakeScaleMatrix(0.3);
	stars->transformation = MakeScaleMatrix(500);


	earth->relativePositionSun = glm::vec3(0, 0, -4);
	earth->relativePositionToPlanet = glm::vec3(0, 0, 0);

	clouds->relativePositionSun = glm::vec3(0, 0, -4);
	clouds->relativePositionToPlanet = glm::vec3(0, 0, 0);



	moon->relativePositionSun = glm::vec3(0, 0, -6);
	moon->relativePositionToPlanet = glm::vec3(0, 0, -2);


	glm::mat4 P =  glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 1000.f);
	glm::mat4 P1 = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 1000.f);
	glm::mat4 P2 = glm::perspective(glm::radians(60.0f), 1.0f, 0.01f, 100.f);
	glm::mat4 P3 = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 100.f);
	glm::mat4 P4 = glm::perspective(glm::radians(90.0f), 1.0f, -1.f, 1.f);

	std::cout << P << std::endl;
	std::cout << P1 << std::endl;
	std::cout << P2 << std::endl;
	std::cout << P3 << std::endl;
	std::cout << P4 << std::endl;





	GPU_Geometry sphere;

	updateGPUGeometry(sphere, earthSphere);

	GLint loc;
	loc = glGetUniformLocation(shader, "T");
	GLint shadeLoc = glGetUniformLocation(shader, "shade");
	GLint radiusLoc = glGetUniformLocation(shader, "radius");
	GLint PosLoc = glGetUniformLocation(shader, "objectPos");

	GLint texture0 = glGetUniformLocation(shader, "sampler");
	GLint texture1 = glGetUniformLocation(shader, "sampler1");
	GLint texture2 = glGetUniformLocation(shader, "sampler2");
	GLint texture3 = glGetUniformLocation(shader, "sampler3");




	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);


		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();


		if (index == 0) {
			a4->camera.ChangeAt(sun->position);
			a4->viewPipeline(shader, sun->transformation, sun->position);
		}
		else {
			a4->camera.ChangeAt(earth->relativePositionSun);
			a4->viewPipeline(shader, sun->transformation, earth->relativePositionSun);
		}

		//a4->viewPipeline(shader,sun->transformation);

		if (animate) {
			//sun
			sun->transformation = glm::rotate(I, sun->rotation * speed, glm::vec3(0, 1, 0)) * sun->transformation;

			//earth
			//earth - rotate areound axis
			earth->transformation = MakeTranslationMatrixVEC(earth->position) * glm::rotate(I, earth->rotation * speed, glm::vec3(0, 1, 1)) * MakeTranslationMatrixVEC(-(earth->position)) * earth->transformation;
			clouds->transformation = MakeTranslationMatrixVEC(clouds->position) * glm::rotate(I, clouds->rotation * speed, glm::vec3(0, 1, 1)) * MakeTranslationMatrixVEC(-(clouds->position)) * clouds->transformation;
			//earth - revolution
			glm::mat4 earthRevo = glm::rotate(I, earth->revolution * speed, glm::vec3(0, 1, 0));
			earth->transformation = earthRevo * earth->transformation;
			earth->position = earth->transformation * glm::vec4(0,0,0,1);
			earth->relativePositionSun = earth->position;

			clouds->transformation = earthRevo * clouds->transformation;
			clouds->position = earth->relativePositionSun;
			clouds->relativePositionSun = clouds->position;



			//moon
			//moon - rotate around axis
			moon->transformation = MakeTranslationMatrixVEC(moon->relativePositionSun) * glm::rotate(I, moon->rotation * speed, glm::vec3(0, 1, 1)) * MakeTranslationMatrixVEC(-(moon->relativePositionSun)) * moon->transformation;


			//moon revolution earth
			moon->transformation = MakeTranslationMatrixVEC(earth->relativePositionSun) * glm::rotate(I, moon->rotation * speed, glm::vec3(0, 1, 0)) * MakeTranslationMatrixVEC(-(earth->relativePositionSun)) * moon->transformation;
			moon->relativePositionToPlanet = moon->relativePositionSun - earth->relativePositionSun;


			//moon - revolution sun
			moon->transformation = earthRevo * moon->transformation;
			moon->position = moon->transformation * glm::vec4(0, 0, 0, 1);
			moon->relativePositionSun = moon->position;


		}

		if (a4->reset) {
			earth->theta = PI;
			moon->theta = PI;

			sun->transformation = I;
			earth->transformation = MakeTranslationMatrixXYZ(0, 0, -4) * glm::rotate(I, PI / 4, glm::vec3(1, 0, 0)) * MakeScaleMatrix(0.6);
			clouds->transformation = MakeTranslationMatrixXYZ(0, 0, -4) * glm::rotate(I, PI / 4, glm::vec3(1, 0, 0)) * MakeScaleMatrix(0.65);
			moon->transformation = MakeTranslationMatrixXYZ(0, 0, -6) * glm::rotate(I, PI / 4, glm::vec3(1, 0, 0)) * MakeScaleMatrix(0.3);
			stars->transformation = MakeScaleMatrix(500);

			earth->relativePositionSun = glm::vec3(0, 0, -4);
			earth->relativePositionToPlanet = glm::vec3(0, 0, 0);

			clouds->relativePositionSun = glm::vec3(0, 0, -4);
			clouds->relativePositionToPlanet = glm::vec3(0, 0, 0);

			moon->relativePositionSun = glm::vec3(0, 0, -6);
			moon->relativePositionToPlanet = glm::vec3(0, 0, -2);



			a4->reset = false;
		}


		sphere.bind();
		//stars->texture.bind2();
		glUniform1i(texture0, 0);
		glUniform1i(texture1, 1);
		glUniform1i(texture2, 2);
		glUniform1i(texture3, 3);


		glActiveTexture(GL_TEXTURE0);
		sun->texture.bind();


		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, p);
		sun->texture.unbind();




		glUniform3fv(PosLoc, 1, glm::value_ptr(moon->position));
		glUniform1f(radiusLoc, 0.3);
		glUniform1f(shadeLoc, 1);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(earth->transformation));

		glActiveTexture(GL_TEXTURE0);
		earth->texture.bind();
		glActiveTexture(GL_TEXTURE1);
		night->texture.bind();
		glActiveTexture(GL_TEXTURE2);
		clouds->texture.bind();
		glActiveTexture(GL_TEXTURE3);
		bump->texture.bind();
		glUniform1f(shadeLoc, 2);
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, p);
		earth->texture.unbind();


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);

		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(clouds->transformation));
		glUniform1f(shadeLoc, 3);
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, p);


		glDisable(GL_BLEND);

		glActiveTexture(GL_TEXTURE0);
		moon->texture.bind();
		glUniform3fv(PosLoc, 1, glm::value_ptr(earth->position));
		glUniform1f(radiusLoc, 0.6);
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(moon->transformation));
		glUniform1f(shadeLoc, 1);

		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, p);
		moon->texture.unbind();





		glUniform1f(shadeLoc, 0);
		stars->texture.bind();
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(stars->transformation));
		glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, p);
		stars->texture.unbind();
		



		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
