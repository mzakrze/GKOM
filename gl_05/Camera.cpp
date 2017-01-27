#include "Camera.h"


Camera::Camera()
{
	cameraPos = glm::vec3(initCameraPosX, initCameraPosY, initCameraPosZ);
	cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3& Camera::getCameraPos(){
	return cameraPos;
}

void Camera::forwardKey(){
	cameraPos += cameraSpeed * cameraFront;
	correctCameraPos();
}

void Camera::backwardKey(){
	cameraPos -= cameraSpeed * cameraFront;
	correctCameraPos();
}

void Camera::leftKey(){
	cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	correctCameraPos();
}

void Camera::rightKey(){
	cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	correctCameraPos();
}

void Camera::setCameraSpeed(double cameraSpeed_){
	cameraSpeed = cameraSpeed_;
}

void Camera::normalizeFront(glm::vec3 &front){
	cameraFront = glm::normalize(front);
}

glm::mat4 Camera::getCameraLookAt(){
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::correctCameraPos(){
	if (cameraPos.x < cameraBoxLowerX){
		cameraPos.x = cameraBoxLowerX;
	}
	else if (cameraPos.x > cameraBoxUpperX){
		cameraPos.x = cameraBoxUpperX;
	}
	if (cameraPos.y < cameraBoxLowerY){
		cameraPos.y = cameraBoxLowerY;
	}
	else if (cameraPos.y > cameraBoxUpperY){
		cameraPos.y = cameraBoxUpperY;
	}
	if (cameraPos.z < cameraBoxLowerZ){
		cameraPos.z = cameraBoxLowerZ;
	}
	else if (cameraPos.z > cameraBoxUpperZ){
		cameraPos.z = cameraBoxUpperZ;
	}
}

Camera::~Camera()
{
}
