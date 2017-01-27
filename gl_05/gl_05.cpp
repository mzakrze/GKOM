#define GLEW_STATIC
#include <map>
#include <GL/glew.h>
#include "shprogram.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include <SOIL.h>
#include <iostream>
#include <concurrent_vector.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

/* zmienne globalne wyznaczaj¹ce rozmiary obiektów i inne parametry*/
const GLuint WIDTH = 800, HEIGHT = 600;

Camera camera;

GLfloat yaw = -90.0f;	
GLfloat pitch = 0.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
GLfloat fov = 45.0f;

bool keys[1024];

GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

const float liftArmAltitudeSize = 0.25f;
float liftArmAngleHorizontal = 90.0f;
const float liftArmAngleVerticalRangeMin = -60.0f;
const float liftArmAngleVerticalRangeMax = 30.0f;
float liftArmAngleVertical = 0.0f;
const float liftArmLength = 5.0f;
const float liftArmOffSet = 0.2f; // wartosc w zakresie (0,1) oznaczajacy jaka czesc ramienia dzwigu stanowi przeciwwaga
const float ropeLenght = 2.0f;		
const float ropeLengthChangeSpeed = 0.01;
float ropeLenghtMultiplier = 1.0f;
float columnYSizeMultiplier = 1.0f;
const float columnYSizeMultiplierRangeMax = 1.8f;
const float columnYSizeMultiplierRangeMin = 0.5f;
const float columnYSize = 8.0f;
const float columnZSize = 0.25f;
const float columnXSize = 0.25;
const float columnOffSet = 0.4f; // wartosc oznaczajaca ile kolumny wystaje nad ramieniem dzwigu
const float containerZSize = 0.3f;
const float containerYSize = 0.3f;
const float containerXSize = 0.3f;
glm::mat4 view_matrix;

glm::vec2 oldMousePosition(0, 0);
glm::vec3 UP(0, 1, 0);
glm::vec3 viewDirection(0, 0, -1);

const float liftArmRotateUpSpeed = 1;
const float liftArmRotateDownSpeed = 1;
const float liftArmRotateHorizontalSpeed = 1;

const float columnExtendSpeed = 0.005f;
const float columnShrinkSpeed = 0.005f;

GLboolean shadows = true;

void printMovementControlls(){
	cout << "Kontrola kamery:" << endl;
	cout << "    W - przesuniêcie w przód" << endl;
	cout << "    S - przesuniêcie w ty³" << endl;
	cout << "    A - przesuniêcie w lewo" << endl;
	cout << "    D - przesuniêcie w prawo" << endl;
	cout << "Kontrola dŸwigu:" << endl;
	cout << "    Q - obrót dŸwigu w lewo" << endl;
	cout << "    E - obrót dŸwigu w prawo" << endl;
	cout << "    R - obrót ramienia w górê" << endl;
	cout << "    F - obrót ramienia w dó³" << endl;
	cout << "    T - wysiêgniêcie dŸwigu w górê" << endl;
	cout << "    G - wysiêgniêcie dŸwigu w dó³" << endl;
	cout << "    Z - opuszczenie liny" << endl;
	cout << "    C - podniesienie liny" << endl;
}

template<typename T1>
void addVertices(vector<T1>& v, T1 t)
{
	v.push_back(t);
}

template<typename T2, typename ...Tlist>
void addVertices(vector<T2>& v, T2 t, Tlist ...list)
{
	v.push_back(t);
	addVertices(v, list...);
}


void addCylinder(vector<GLfloat>& verticesCylinder, GLfloat cylinder_height, GLfloat cylinder_radius, GLfloat nr_of_points_cylinder,
	GLfloat centerX, GLfloat centerY, GLfloat centerZ, GLfloat cx, GLfloat cy, GLfloat cz)
{
	GLfloat x0, y0, z0, x1, y1, z1;

	for (int i = 0; i <= nr_of_points_cylinder; ++i)
	{
		GLfloat u0 = i / (GLfloat)nr_of_points_cylinder;
		GLfloat u1 = (i + 1) / (GLfloat)nr_of_points_cylinder;
		u0 *= 2 * 3.14f;
		u1 *= 2 * 3.14f;
		//Where the cylinder is in the x and z positions (3D space) 
		x0 = centerX
			+ cylinder_radius*cos(u0);
		y0 = centerY
			+ cylinder_radius*sin(u0);
		z0 = centerZ;
		x1 = centerX
			+ cylinder_radius*cos(u1);
		y1 = centerY
			+ cylinder_radius*sin(u1);
		z1 = centerZ;

		float greyColor = 0.333f;

		addVertices(verticesCylinder, centerX, centerY, centerZ, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x0, y0, z0, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x1, y1, z1, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, centerX, centerY, centerZ + cylinder_height, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x0, y0, z0 + cylinder_height, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x1, y1, z1 + cylinder_height, greyColor, greyColor, greyColor);

		addVertices(verticesCylinder, x0, y0, z0, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x1, y1, z1, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x1, y1, z1 + cylinder_height, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x0, y0, z0, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x0, y0, z0 + cylinder_height, greyColor, greyColor, greyColor);
		addVertices(verticesCylinder, x1, y1, z1 + cylinder_height, greyColor, greyColor, greyColor);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.7;	// Change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	camera.normalizeFront(front); // cameraFront = glm::normalize(front);
}


GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

void do_movement()
{
	GLfloat timeTreshHold = 0.01;
	static GLfloat prevTime = glfwGetTime();
	if (glfwGetTime() - prevTime < timeTreshHold)
		return;

	prevTime = glfwGetTime();
	// GLfloat cameraSpeed = 50.0f * deltaTime;
	camera.setCameraSpeed(50.0f * deltaTime);
	if (keys[GLFW_KEY_W])
		camera.forwardKey();
	if (keys[GLFW_KEY_S])
		camera.backwardKey();
	if (keys[GLFW_KEY_A])
		camera.leftKey();
	if (keys[GLFW_KEY_D])
		camera.rightKey();
	if (keys[GLFW_KEY_Q])
		liftArmAngleHorizontal += liftArmRotateHorizontalSpeed;
	if (keys[GLFW_KEY_E])
		liftArmAngleHorizontal -= liftArmRotateHorizontalSpeed;
	if (keys[GLFW_KEY_F] && liftArmAngleVertical < liftArmAngleVerticalRangeMax)
		liftArmAngleVertical += liftArmRotateUpSpeed;
	if (keys[GLFW_KEY_R] && liftArmAngleVertical > liftArmAngleVerticalRangeMin)
		liftArmAngleVertical -= liftArmRotateDownSpeed;
	if (keys[GLFW_KEY_T])
		columnYSizeMultiplier += columnExtendSpeed;
	if (keys[GLFW_KEY_G])
		columnYSizeMultiplier -= columnShrinkSpeed;
	if (keys[GLFW_KEY_Z])
		ropeLenghtMultiplier -= ropeLengthChangeSpeed;
	if (keys[GLFW_KEY_C])
		ropeLenghtMultiplier += ropeLengthChangeSpeed;
}

GLuint indicesForCuboid[] = {
	// przod
	5, 4, 7,
	4, 7, 6,
	// tyl
	0, 1, 2,
	1, 2, 3,
	// prawy
	4, 0, 6,
	6, 0, 2,
	// lewy
	5, 1, 7,
	7, 1, 3,
	// gora
	5, 1, 4,
	4, 1, 0,
	// dol
	7, 6, 3,
	3, 6, 2
};

/**
sterowanie:
w - do przodu
s - do ty³u
a - w lewo
d - w prawo
e - obrót dxwigu w prawo
q - obrot dzwigu w lewo
r - wydluzenie wysiegnika
f - skrocenie wysiegnika
t - wysiegnik w gore
z - skrocenie liny
c - wydluzenie liny
g- wysiegnik w dol
myszka - rozgl¹danie siê
*/
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

GLuint LoadMipmapTexture(GLuint texId, const char* fname)
{
	int width, height;
	unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGB);
	if (image == nullptr)
		throw exception("Failed to load texture file");

	GLuint texture;
	glGenTextures(1, &texture);

	glActiveTexture(texId);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

void fillArrayToCreateCuboid(GLfloat array[], float width, float height, float depth){
	GLfloat workArray[] =
	{
		// x y z								normals					textures coords
		-width / 2, -height / 2, -depth / 2,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f, // Bottom-left
		width / 2, height / 2, -depth / 2,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f, // top-right
		width / 2, -height / 2, -depth / 2,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f, // bottom-right         
		width / 2, height / 2, -depth / 2,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f, // top-right
		-width / 2, -height / 2, -depth / 2,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f, // bottom-left
		-width / 2, height / 2, -depth / 2,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f, // top-left
		// Front face
		-width / 2, -height / 2, depth / 2,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f, // bottom-left
		width / 2, -height / 2, depth / 2,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, // bottom-right
		width / 2, height / 2, depth / 2,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f, // top-right
		width / 2, height / 2, depth / 2,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f, // top-right
		-width / 2, height / 2, depth / 2,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f, // top-left
		-width / 2, -height / 2, depth / 2,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f, // bottom-left
		// Left face
		-width / 2, height / 2, depth / 2,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f, // top-right
		-width / 2, height / 2, -depth / 2,		-1.0f, 0.0f, 0.0f,		1.0f, 1.0f, // top-left
		-width / 2, -height / 2, -depth / 2,	-1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // bottom-left
		-width / 2, -height / 2, -depth / 2,	-1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // bottom-left
		-width / 2, -height / 2, depth / 2,		-1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // bottom-right
		-width / 2, height / 2, depth / 2,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f, // top-right
		// Right face
		width / 2, height / 2, depth / 2,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f, // top-left
		width / 2, -height / 2, -depth / 2,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // bottom-right
		width / 2, height / 2, -depth / 2,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f, // top-right         
		width / 2, -height / 2, -depth / 2,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, // bottom-right
		width / 2, height / 2, depth / 2,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f, // top-left
		width / 2, -height / 2, depth / 2,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, // bottom-left     
		// Bottom face
		-width / 2, -height / 2, -depth / 2,	0.0f, -1.0f, 0.0f,		0.0f, 1.0f, // top-right
		width / 2, -height / 2, -depth / 2,		0.0f, -1.0f, 0.0f,		1.0f, 1.0f, // top-left
		width / 2, -height / 2, depth / 2,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f, // bottom-left
		width / 2, -height / 2, depth / 2,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f, // bottom-left
		-width / 2, -height / 2, depth / 2,		0.0f, -1.0f, 0.0f,		0.0f, 0.0f, // bottom-right
		-width / 2, -height / 2, -depth / 2,	0.0f, -1.0f, 0.0f,		0.0f, 1.0f, // top-right
		// Top face
		-width / 2, height / 2, -depth / 2,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f, // top-left
		width / 2, height / 2, depth / 2,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f, // bottom-right
		width / 2, height / 2, -depth / 2,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f, // top-right     
		width / 2, height / 2, depth / 2,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f, // bottom-right
		-width / 2, height / 2, -depth / 2,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f, // top-left
		-width / 2, height / 2, depth / 2,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f  // bottom-left   
	};
	memcpy(array, workArray, 288 * sizeof(GLfloat));
}

void fillArrayToCreateSkyBox(GLfloat skyboxVertices[], float skyboxRadius){
	GLfloat workArray[] = {       
		-skyboxRadius, skyboxRadius, -skyboxRadius,
		-skyboxRadius, -skyboxRadius, -skyboxRadius,
		skyboxRadius, -skyboxRadius, -skyboxRadius,
		skyboxRadius, -skyboxRadius, -skyboxRadius,
		skyboxRadius, skyboxRadius, -skyboxRadius,
		-skyboxRadius, skyboxRadius, -skyboxRadius,

		-skyboxRadius, -skyboxRadius, skyboxRadius,
		-skyboxRadius, -skyboxRadius, -skyboxRadius,
		-skyboxRadius, skyboxRadius, -skyboxRadius,
		-skyboxRadius, skyboxRadius, -skyboxRadius,
		-skyboxRadius, skyboxRadius, skyboxRadius,
		-skyboxRadius, -skyboxRadius, skyboxRadius,

		skyboxRadius, -skyboxRadius, -skyboxRadius,
		skyboxRadius, -skyboxRadius, skyboxRadius,
		skyboxRadius, skyboxRadius, skyboxRadius,
		skyboxRadius, skyboxRadius, skyboxRadius,
		skyboxRadius, skyboxRadius, -skyboxRadius,
		skyboxRadius, -skyboxRadius, -skyboxRadius,

		-skyboxRadius, -skyboxRadius, skyboxRadius,
		-skyboxRadius, skyboxRadius, skyboxRadius,
		skyboxRadius, skyboxRadius, skyboxRadius,
		skyboxRadius, skyboxRadius, skyboxRadius,
		skyboxRadius, -skyboxRadius, skyboxRadius,
		-skyboxRadius, -skyboxRadius, skyboxRadius,

		-skyboxRadius, skyboxRadius, -skyboxRadius,
		skyboxRadius, skyboxRadius, -skyboxRadius,
		skyboxRadius, skyboxRadius, skyboxRadius,
		skyboxRadius, skyboxRadius, skyboxRadius,
		-skyboxRadius, skyboxRadius, skyboxRadius,
		-skyboxRadius, skyboxRadius, -skyboxRadius,

		-skyboxRadius, -skyboxRadius, -skyboxRadius,
		-skyboxRadius, -skyboxRadius, skyboxRadius,
		skyboxRadius, -skyboxRadius, -skyboxRadius,
		skyboxRadius, -skyboxRadius, -skyboxRadius,
		-skyboxRadius, -skyboxRadius, skyboxRadius,
		skyboxRadius, -skyboxRadius, skyboxRadius
	};

	memcpy(skyboxVertices, workArray, 108 * sizeof(GLfloat));
}

GLuint texture0;
GLuint texture1;
GLuint platfomTexture;
GLuint liftArmTexture;
GLuint groundTexture;
GLuint containerTexture;

GLuint columnVBO, columnEBO, columnVAO;
GLuint floorVBO, floorEBO, floorVAO;
GLuint liftArmVBO, liftArmEBO, liftArmVAO;
GLuint ropeVBO, ropeEBO, ropeVAO;
GLuint containerVBO, containerEBO, containerVAO;
GLuint cylinderVAO, cylinderVBO;

void RenderScene(ShaderProgram &shader){

	// kolumna
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, liftArmTexture);
	glm::mat4 columnModelMatrix;
	columnModelMatrix = glm::rotate(columnModelMatrix, glm::radians(liftArmAngleHorizontal), glm::vec3(0, 1, 0));
	columnModelMatrix = glm::translate(columnModelMatrix, glm::vec3(0, columnYSize * columnYSizeMultiplier / 2, 0));
	columnModelMatrix = glm::scale(columnModelMatrix, glm::vec3(1, columnYSizeMultiplier, 1));
	glUniformMatrix4fv(glGetUniformLocation(shader.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(columnModelMatrix));
	glBindVertexArray(columnVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36 * 16);
	glBindVertexArray(0);

	// ramie dzwigu
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, liftArmTexture);
	glm::mat4 liftArmModelMatrix;
	liftArmModelMatrix = glm::rotate(liftArmModelMatrix, glm::radians(liftArmAngleHorizontal), glm::vec3(0, 1, 0));
	liftArmModelMatrix = glm::translate(liftArmModelMatrix, glm::vec3(0, columnYSize * columnYSizeMultiplier - columnOffSet, 0));
	liftArmModelMatrix = glm::rotate(liftArmModelMatrix, glm::radians(liftArmAngleVertical), glm::vec3(1, 0, 0));
	liftArmModelMatrix = glm::translate(liftArmModelMatrix, glm::vec3(0, 0, liftArmLength * (0.5 - liftArmOffSet)));
	liftArmModelMatrix = glm::rotate(liftArmModelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(liftArmModelMatrix));
	glBindVertexArray(liftArmVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36 * 8);
	glBindVertexArray(0);

	// lina
	float topLiftArmY = (columnYSizeMultiplier * columnYSize - columnOffSet) + liftArmLength * (1 - liftArmOffSet) * sin(glm::radians(-liftArmAngleVertical));
	float topLiftArmX = liftArmLength * (1 - liftArmOffSet) * cos(glm::radians(-liftArmAngleVertical)) * sin(glm::radians(liftArmAngleHorizontal));
	float topLiftArmZ = liftArmLength * (1 - liftArmOffSet) * cos(glm::radians(-liftArmAngleVertical)) * cos(glm::radians(liftArmAngleHorizontal));
	glm::mat4 ropeModelMatrix;
	ropeModelMatrix = glm::translate(ropeModelMatrix, glm::vec3(topLiftArmX, topLiftArmY - ropeLenght * ropeLenghtMultiplier, topLiftArmZ));
	ropeModelMatrix = glm::scale(ropeModelMatrix, glm::vec3(1, ropeLenghtMultiplier, 1));
	glUniformMatrix4fv(glGetUniformLocation(shader.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(ropeModelMatrix));
	glBindVertexArray(ropeVAO);
	glLineWidth(5.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);

	// kontener
	glBindTexture(GL_TEXTURE_2D, containerTexture);
	glm::mat4 containerModelMatrix;
	containerModelMatrix = glm::translate(containerModelMatrix, glm::vec3(topLiftArmX, topLiftArmY - ropeLenght * ropeLenghtMultiplier, topLiftArmZ));
	containerModelMatrix = glm::rotate(containerModelMatrix, glm::radians(liftArmAngleHorizontal), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(glGetUniformLocation(shader.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(containerModelMatrix));
	glBindVertexArray(containerVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36 * 8);
	glBindVertexArray(0);

	// platforma

	// cylinder
	


	// podloga
	glm::mat4 model;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, groundTexture);
	glUniformMatrix4fv(glGetUniformLocation(shader.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(model));
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}


int main()
{
	if (glfwInit() != GL_TRUE)
	{
		cout << "GLFW initialization failed" << endl;
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	try
	{
		GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GKOM - Projekt", nullptr, nullptr);
		if (window == nullptr)
			throw exception("GLFW window not created");
		glfwMakeContextCurrent(window);
		glfwSetKeyCallback(window, key_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
			throw exception("GLEW Initialization failed");

		glViewport(0, 0, WIDTH, HEIGHT);

		
		GLint nrAttributes;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
		cout << "Max vertex attributes allowed: " << nrAttributes << std::endl;
		glGetIntegerv(GL_MAX_TEXTURE_COORDS, &nrAttributes);
		cout << "Max texture coords allowed: " << nrAttributes << std::endl;

		printMovementControlls();


		ShaderProgram cubemapProgram("cubemap.vert", "cubemap.frag");
		ShaderProgram shadow_mappingShader("shadow_mapping.vs", "shadow_mapping.frag");
		ShaderProgram shadow_mapping_depthShader("shadow_mapping_depth.vs", "shadow_mapping_depth.frag");
		ShaderProgram cylinderProgram("onlyColor.vert", "onlyColor.frag");

		shadow_mappingShader.Use();
		glUniform1i(glGetUniformLocation(shadow_mappingShader.get_programID(), "diffuseTexture"), 0);
		glUniform1i(glGetUniformLocation(shadow_mappingShader.get_programID(), "shadowMap"), 1);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		GLfloat columnVertices[288];
		fillArrayToCreateCuboid(columnVertices, columnXSize, columnYSize, columnZSize);
		
		GLfloat liftArmVertices[288];
		fillArrayToCreateCuboid(liftArmVertices, liftArmAltitudeSize, liftArmLength, liftArmAltitudeSize);

		GLfloat containerVertices[288];
		fillArrayToCreateCuboid(containerVertices, containerXSize, containerYSize, containerZSize);

		GLfloat platformAltitudeSize = 1.0f, platformYSize = 0.5f;
		GLfloat platformVertices[288];
		fillArrayToCreateCuboid(platformVertices, platformAltitudeSize, platformYSize, platformAltitudeSize);
		GLfloat ropeVertices[16] = {
			0.0f, 0.0, 0.0f,			1.0f, 1.0f,
			0.0f, ropeLenght, 0.0f,		1.0f, 1.0f,
		};

		GLfloat ropeIndices[] = {
			0, 1
		};

		GLfloat skyboxRadius = 50.0f;
		GLfloat skyboxVertices[108];
		fillArrayToCreateSkyBox(skyboxVertices, skyboxRadius);
		// Setup skybox VAO
		GLuint skyboxVAO, skyboxVBO;
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);
		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glBindVertexArray(0);
		// Cubemap (Skybox)
		vector<const GLchar*> faces;
		faces.push_back("nalovardo/posx.jpg");
		faces.push_back("nalovardo/negx.jpg");
		faces.push_back("nalovardo/posy.jpg");
		faces.push_back("nalovardo/negy.jpg");
		faces.push_back("nalovardo/posz.jpg");
		faces.push_back("nalovardo/negz.jpg");
		GLuint skyboxTexture = loadCubemap(faces);


		glGenVertexArrays(1, &ropeVAO);
		glGenBuffers(1, &ropeEBO);
		glGenBuffers(1, &ropeVBO);
		glBindVertexArray(ropeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, ropeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ropeVertices), ropeVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0); // vertex geometry data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // vertex texture data
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);


		float floorSize = 700;
		GLfloat floorVertices[] = {
			// xyz								normal		texture
			floorSize / 2, 0, -floorSize / 2,	0, 1, 0,	0, 0,
			floorSize / 2, 0, floorSize / 2,	0, 1, 0,	10, 0,
			-floorSize / 2, 0, floorSize / 2,	0, 1, 0,	10, 10,
			floorSize / 2, 0, -floorSize / 2,	0, 1, 0,	0, 0,
			-floorSize / 2, 0, floorSize / 2,	0, 1, 0,	10, 10,
			-floorSize / 2, 0, -floorSize / 2,	0, 1, 0,	10, 0
		};
		glGenVertexArrays(1, &floorVAO);
		glGenBuffers(1, &floorVBO);
		glBindVertexArray(floorVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // vertex geometry data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // vertex normal coordinates
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // vertex texture coordinates
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);


		glGenVertexArrays(1, &containerVAO);
		glGenBuffers(1, &containerVBO);
		glBindVertexArray(containerVAO);
		glBindBuffer(GL_ARRAY_BUFFER, containerVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(containerVertices), containerVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // vertex geometry data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // vertex normals data
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // vertex texture data
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);


		vector<GLfloat> cylinderVertices;
		addCylinder(cylinderVertices, 0.5f, 1.0f, 720.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		glGenVertexArrays(1, &cylinderVAO);
		glGenBuffers(1, &cylinderVBO);
		glBindVertexArray(cylinderVAO);
		glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
		glBufferData(GL_ARRAY_BUFFER, cylinderVertices.size()*sizeof(float), &cylinderVertices.front(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glBindVertexArray(0);


		glGenVertexArrays(1, &liftArmVAO);
		glGenBuffers(1, &liftArmEBO);
		glGenBuffers(1, &liftArmVBO);
		glBindVertexArray(liftArmVAO);
		glBindBuffer(GL_ARRAY_BUFFER, liftArmVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(liftArmVertices), liftArmVertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, liftArmEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesForCuboid), indicesForCuboid, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // vertex geometry data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // vertex normals data
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // vertex texture data
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);


		glGenVertexArrays(1, &columnVAO);
		glGenBuffers(1, &columnVBO);
		glBindVertexArray(columnVAO);
		glBindBuffer(GL_ARRAY_BUFFER, columnVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(columnVertices), columnVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // vertex geometry data
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); // vertex normals data
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat))); // vertex texture data
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);


		// Set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		platfomTexture = LoadMipmapTexture(GL_TEXTURE2, "concrete.png");
		liftArmTexture = LoadMipmapTexture(GL_TEXTURE3, "rusztowanie.png");
		groundTexture = LoadMipmapTexture(GL_TEXTURE4, "ground.png");
		containerTexture = LoadMipmapTexture(GL_TEXTURE5, "container.png");
		

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glEnable(GL_DEPTH_TEST);


		GLuint depthMapFBO;
		glGenFramebuffers(1, &depthMapFBO);
		GLuint depthMap;
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// main event loop
		while (!glfwWindowShouldClose(window))
		{
			GLfloat currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			glfwPollEvents();
			do_movement();

			glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shadow_mapping_depthShader.Use();
			static float zNear = 0.1f;
			static float zFar = 100.0f;
			static float lightPosX = -8.0f;
			static float lightPosY = 10.0f;
			static float lightPosZ = 12.0f;
			glm::mat4 lightProjection;
			lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, zNear, zFar);
			//lightProjection = glm::perspective(fov, (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, zNear, 100.0f); 
			glm::vec3 lightPos(lightPosX, lightPosY, lightPosZ);
			glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
			glm::mat4 lightSpaceMatrix = lightProjection * lightView;
			glUniformMatrix4fv(glGetUniformLocation(shadow_mapping_depthShader.get_programID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			RenderScene(shadow_mapping_depthShader);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);


			glViewport(0, 0, WIDTH, HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shadow_mappingShader.Use();
			glm::mat4 projection_matrix = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, zNear, zFar);
			view_matrix = camera.getCameraLookAt();
			glUniform1i(glGetUniformLocation(shadow_mappingShader.get_programID(), "shadows"), shadows);
			glUniformMatrix4fv(glGetUniformLocation(shadow_mappingShader.get_programID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
			glUniformMatrix4fv(glGetUniformLocation(shadow_mappingShader.get_programID(), "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
			glUniform3fv(glGetUniformLocation(shadow_mappingShader.get_programID(), "lightPos"), 1, &lightPos[0]);
			glUniform3fv(glGetUniformLocation(shadow_mappingShader.get_programID(), "viewPos"), 1, &camera.getCameraPos()[0]);
			glUniformMatrix4fv(glGetUniformLocation(shadow_mappingShader.get_programID(), "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			RenderScene(shadow_mappingShader);



			cylinderProgram.Use();
			glm::mat4 cylinderModel;
			cylinderModel = glm::translate(cylinderModel, glm::vec3(0, 1, 0));
			cylinderModel = glm::rotate(cylinderModel, glm::radians(90.0f), glm::vec3(1, 0, 0));
			glUniformMatrix4fv(glGetUniformLocation(cylinderProgram.get_programID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
			glUniformMatrix4fv(glGetUniformLocation(cylinderProgram.get_programID(), "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
			glUniformMatrix4fv(glGetUniformLocation(cylinderProgram.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(cylinderModel));
			glBindVertexArray(cylinderVAO);
			glDrawArrays(GL_TRIANGLES, 0, 720 * 3 * 4);
			glBindVertexArray(0);



			// skybox
			cubemapProgram.Use();
			glm::mat4 skyboxModel;
			skyboxModel = glm::translate(skyboxModel, camera.getCameraPos()); // cameraPos
			glUniformMatrix4fv(glGetUniformLocation(cubemapProgram.get_programID(), "model"), 1, GL_FALSE, glm::value_ptr(skyboxModel));
			glUniformMatrix4fv(glGetUniformLocation(cubemapProgram.get_programID(), "view"), 1, GL_FALSE, glm::value_ptr(view_matrix));
			glUniformMatrix4fv(glGetUniformLocation(cubemapProgram.get_programID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection_matrix));
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glUniform1i(glGetUniformLocation(cubemapProgram.get_programID(), "cubemap"), 0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glDepthFunc(GL_LESS);


			glfwSwapBuffers(window);
		}
		glDeleteVertexArrays(1, &columnVAO);
		glDeleteBuffers(1, &columnVBO);
		glDeleteBuffers(1, &columnEBO);
		glDeleteVertexArrays(1, &liftArmVAO);
		glDeleteBuffers(1, &liftArmVBO);
		glDeleteBuffers(1, &liftArmEBO);
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
	}
	glfwTerminate();

	return 0;
}

