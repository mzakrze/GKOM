
#include <iostream>
#include <cmath>
#include <SOIL.h>
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "shprogram.h"

using namespace std;
// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
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
// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

GLfloat yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch = 0.0f;
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
GLfloat fov = 45.0f;
// Camera
glm::vec3 cameraPos = glm::vec3(-5.0f, 5.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool    keys[1024];

// Light attributes
float lightPosX = 1;
float lightPosY = 1;
float lightPosZ = 1;

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// The MAIN function, from here we start the application and run the game loop
int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);


	// Build and compile our shader program
	ShaderProgram lightingShader("lightingShader.vert", "lightingShader.frag");
	ShaderProgram lampShader("lampShader.vert", "lampShader.frag");

	// Set up vertex data (and buffer(s)) and attribute pointers
	float cuboidXSize = 2;
	float cuboidYSize = 2;
	float cuboidZSize = 2;
	GLfloat vertices[] = {
		-cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 0.0f, -1.0f,	0, 1,
		cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 0.0f, -1.0f,		1, 1,
		cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 0.0f, -1.0f,		0, 0,
		cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 0.0f, -1.0f,		0, 1,
		-cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 0.0f, -1.0f,		1, 1,
		-cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 0.0f, -1.0f,	1, 0,

		-cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 0.0f, 0.0f, 1.0f,		0,1,
		cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 0.0f, 0.0f, 1.0f,		1,1,
		cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 0.0f, 0.0f, 1.0f,		0,1,
		cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 0.0f, 0.0f, 1.0f,		1,0,
		-cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 0.0f, 0.0f, 1.0f,		0,0,
		-cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 0.0f, 0.0f, 1.0f,		1,1,

		-cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, -1.0f, 0.0f, 0.0f,		0,1,
		-cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, -1.0f, 0.0f, 0.0f,		1,1,
		-cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, -1.0f, 0.0f, 0.0f,	0,0,
		-cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, -1.0f, 0.0f, 0.0f,	1,0,
		-cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, -1.0f, 0.0f, 0.0f,		0,1,
		-cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, -1.0f, 0.0f, 0.0f,		1,1,

		cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 1.0f, 0.0f, 0.0f,		1,1,
		cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 1.0f, 0.0f, 0.0f,		1,0,
		cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 1.0f, 0.0f, 0.0f,		0,0,
		cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 1.0f, 0.0f, 0.0f,		1,1,
		cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 1.0f, 0.0f, 0.0f,		1,0,
		cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 1.0f, 0.0f, 0.0f,		0,1,

		-cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 0.0f, -1.0f, 0.0f,	0,0,
		cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 0.0f, -1.0f, 0.0f,		1,1,
		cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 0.0f, -1.0f, 0.0f,		0,1,
		cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 0.0f, -1.0f, 0.0f,		1,1,
		-cuboidXSize / 2, -cuboidYSize / 2, cuboidZSize / 2, 0.0f, -1.0f, 0.0f,		0,1,
		-cuboidXSize / 2, -cuboidYSize / 2, -cuboidZSize / 2, 0.0f, -1.0f, 0.0f,	1,0,

		-cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 1.0f, 0.0f,		0,0,
		cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 1.0f, 0.0f,		1,1,
		cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 0.0f, 1.0f, 0.0f,		0,1,
		cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 0.0f, 1.0f, 0.0f,		1,0,
		-cuboidXSize / 2, cuboidYSize / 2, cuboidZSize / 2, 0.0f, 1.0f, 0.0f,		0,0,
		-cuboidXSize / 2, cuboidYSize / 2, -cuboidZSize / 2, 0.0f, 1.0f, 0.0f,		0,1,
	};

	GLfloat floorVertices[] = {
		// position	normal		texture
		0, 0, 0, 0, 1, 0, 0, 0,
		2, 0, 2, 0, 1, 0, 0, 1,
		2, 0, 0, 0, 1, 0, 1, 1,
	};

	GLuint floorVAO, floorVBO;
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
	glBindVertexArray(floorVAO);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// First, set the container's VAO (and VBO)
	GLuint VBO, containerVAO;
	glGenVertexArrays(1, &containerVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(containerVAO);
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Texture attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// Then, we set the light's VAO (VBO stays the same. After all, the vertices are the same for the light object (also a 3D cube))
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	// We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Set the vertex attributes (only position data for the lamp))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the normal vectors
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLuint liftArmTexture = LoadMipmapTexture(GL_TEXTURE0, "liftArm.png");

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::vec3 lightPos(lightPosX, lightPosY, lightPosZ);
		static float radius = 2.0f;
		static float lightAngle = 0.0f;
		lightAngle += 0.05;
		lightPosX = radius * sin(glm::radians(lightAngle));
		lightPosX = radius * cos(glm::radians(lightAngle));

		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();
		GLint objectColorLoc = glGetUniformLocation(lightingShader.get_programID(), "objectColor");
		GLint lightColorLoc = glGetUniformLocation(lightingShader.get_programID(), "lightColor");
		GLint lightPosLoc = glGetUniformLocation(lightingShader.get_programID(), "lightPos");
		GLint viewPosLoc = glGetUniformLocation(lightingShader.get_programID(), "viewPos");
		glUniform3f(objectColorLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);


		// Create camera transformations
		glm::mat4 view;
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(fov, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.get_programID(), "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.get_programID(), "view");
		GLint projLoc = glGetUniformLocation(lightingShader.get_programID(), "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Draw the container (using container's vertex attributes)
		glBindVertexArray(containerVAO);
		glBindTexture(GL_TEXTURE_2D, liftArmTexture);
		glUniform1i(glGetUniformLocation(lightingShader.get_programID(), "Texture0"), 0);
		glm::mat4 model;
		static float angle = 0.0f;
		angle += 0.01f;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(1, 1, 0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);


		glBindVertexArray(floorVAO);
		model = glm::mat4();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);



		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.get_programID(), "model");
		viewLoc = glGetUniformLocation(lampShader.get_programID(), "view");
		projLoc = glGetUniformLocation(lampShader.get_programID(), "projection");
		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
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

void do_movement()
{
	GLfloat timeTreshHold = 0.01;
	static GLfloat prevTime = glfwGetTime();
	if (glfwGetTime() - prevTime < timeTreshHold)
		return;
	prevTime = glfwGetTime();
	GLfloat cameraSpeed = 50.0f * deltaTime;
	if (keys[GLFW_KEY_W])
		cameraPos += cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_S])
		cameraPos -= cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_A])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keys[GLFW_KEY_D])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
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
	cameraFront = glm::normalize(front);
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