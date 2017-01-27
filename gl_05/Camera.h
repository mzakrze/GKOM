#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
class Camera
{
private:
	const GLfloat initCameraPosX = 1.0f; 
	const GLfloat initCameraPosY = 1.0f;
	const GLfloat initCameraPosZ = 1.0f;
	const GLfloat cameraBoxLowerX = -10.0f;
	const GLfloat cameraBoxUpperX = 10.0f;
	const GLfloat cameraBoxLowerZ = -10.0f;
	const GLfloat cameraBoxUpperZ = 10.0f;
	const GLfloat cameraBoxLowerY = 0.11f;
	const GLfloat cameraBoxUpperY = 10.0f;
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	GLfloat cameraSpeed;
	void correctCameraPos();
public:
	void forwardKey();
	void backwardKey();
	void leftKey();
	void rightKey();
	void setCameraSpeed(double cameraSpeed);
	void normalizeFront(glm::vec3 &front);
	glm::vec3& getCameraPos();
	glm::mat4 getCameraLookAt();
	Camera();
	~Camera();
};

