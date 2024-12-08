// ViewOBJModel.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <Windows.h>
#include <locale>
#include <codecvt>

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 

#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Model.h"
#include "FlyingCube.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//pus de mine
bool FPS = true;

enum ECameraMovementType
{
	UNKNOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
private:
	// Default camera values
	const float zNEAR = 0.1f;
	const float zFAR = 500.f;
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float FOV = 45.0f;
	glm::vec3 startPosition;

public:
	Camera(const int width, const int height, const glm::vec3& position)
	{
		startPosition = position;
		Set(width, height, position);
	}

	void Set(const int width, const int height, const glm::vec3& position)
	{
		this->isPerspective = true;
		this->yaw = YAW;
		this->pitch = PITCH;

		this->FoVy = FOV;
		this->width = width;
		this->height = height;
		this->zNear = zNEAR;
		this->zFar = zFAR;

		this->worldUp = glm::vec3(0, 1, 0);
		this->position = position;

		lastX = width / 2.0f;
		lastY = height / 2.0f;
		bFirstMouseMove = true;

		UpdateCameraVectors();
	}

	void Reset(const int width, const int height)
	{
		Set(width, height, startPosition);
	}

	void Reshape(int windowWidth, int windowHeight)
	{
		width = windowWidth;
		height = windowHeight;

		// define the viewport transformation
		glViewport(0, 0, windowWidth, windowHeight);
	}

	const glm::mat4 GetViewMatrix() const
	{
		// Returns the View Matrix
		return glm::lookAt(position, position + forward, up);
	}

	const glm::vec3 GetPosition() const
	{
		return position;
	}

	const glm::mat4 GetProjectionMatrix() const
	{
		glm::mat4 Proj = glm::mat4(1);
		if (isPerspective) {
			float aspectRatio = ((float)(width)) / height;
			Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
		}
		else {
			float scaleFactor = 2000.f;
			Proj = glm::ortho<float>(
				-width / scaleFactor, width / scaleFactor,
				-height / scaleFactor, height / scaleFactor, -zFar, zFar);
		}
		return Proj;
	}

	void ProcessKeyboard(ECameraMovementType direction, float deltaTime)
	{
		float velocity = (float)(cameraSpeedFactor * deltaTime);
		switch (direction) {
		case ECameraMovementType::FORWARD:
			position += forward * velocity;
			break;
		case ECameraMovementType::BACKWARD:
			position -= forward * velocity;
			break;
		case ECameraMovementType::LEFT:
			position -= right * velocity;
			break;
		case ECameraMovementType::RIGHT:
			position += right * velocity;
			break;
		case ECameraMovementType::UP:
			position += up * velocity;
			break;
		case ECameraMovementType::DOWN:
			position -= up * velocity;
			break;
		}
	}

	void MouseControl(float xPos, float yPos)
	{
		if (bFirstMouseMove) {
			lastX = xPos;
			lastY = yPos;
			bFirstMouseMove = false;
		}

		float xChange = xPos - lastX;
		float yChange = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
			return;
		}
		xChange *= mouseSensitivity;
		yChange *= mouseSensitivity;

		ProcessMouseMovement(xChange, yChange);
	}
public:
	void ProcessMouseScroll(float yOffset)
	{
		if (FoVy >= 1.0f && FoVy <= 90.0f) {
			FoVy -= yOffset;
		}
		if (FoVy <= 1.0f)
			FoVy = 1.0f;
		if (FoVy >= 90.0f)
			FoVy = 90.0f;
	}

private:
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
	{
		yaw += xOffset;
		pitch += yOffset;

		//std::cout << "yaw = " << yaw << std::endl;
		//std::cout << "pitch = " << pitch << std::endl;

		// Avem grijã sã nu ne dãm peste cap
		if (constrainPitch) {
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		// Se modificã vectorii camerei pe baza unghiurilor Euler
		UpdateCameraVectors();
	}

	void UpdateCameraVectors()
	{
		// Calculate the new forward vector
		this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward.y = sin(glm::radians(pitch));
		this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward = glm::normalize(this->forward);
		// Also re-calculate the Right and Up vector
		right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up = glm::normalize(glm::cross(right, forward));
	}

public:
	void setPosition(const glm::vec3& position)
	{
		this->position = position;
	}

protected:
	const float cameraSpeedFactor = 250.0f;
	const float mouseSensitivity = 0.1f;

	// Perspective properties
	float zNear;
	float zFar;
	float FoVy;
	int width;
	int height;
	bool isPerspective;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	// Euler Angles
	float yaw;
	float pitch;

	bool bFirstMouseMove = true;
	float lastX = 0.f, lastY = 0.f;
};

GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
Camera* pCamera = nullptr;

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {

	}

	if (!FPS) //daca sunt in tren tastele de miscare nu merg
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
			pCamera->ProcessKeyboard(UP, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
			pCamera->ProcessKeyboard(DOWN, (float)deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);

	}


	//Puse de la mine
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		FPS = true;
	}

	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
	{
		FPS = false;
	}


}

int main()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Lab 7", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	// first, configure the cube's VAO (and VBO)
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 3.0, 3.0));

	glm::vec3 lightPos(0.0f, 2.0f, 1.0f);
	glm::vec3 cubePos(0.0f, 5.0f, 1.0f);
	glm::vec3 trainPos(0.0f, 0.0f, 0.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	Shader lightingShader((currentPath + "\\Shaders\\PhongLight.vs").c_str(), (currentPath + "\\Shaders\\PhongLight.fs").c_str());
	Shader lightingWithTextureShader((currentPath + "\\Shaders\\PhongLightWithTexture.vs").c_str(), (currentPath + "\\Shaders\\PhongLightWithTexture.fs").c_str());
	Shader lampShader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());

	std::string objFileName = (currentPath + "\\Models\\FlyingCube.obj");
	FlyingCube flyingCubeModel(objFileName, false);

	std::string piratObjFileName = (currentPath + "\\Models\\Pirat\\Pirat.obj");
	Model piratObjModel(piratObjFileName, false);


	std::string GrassLawnFileName = (currentPath + "\\Models\\GrassLawn\\grassLawn.obj");
	Model GrassLawnModel(GrassLawnFileName, false);

	std::string trainObjFileName = (currentPath + "\\Models\\Train\\train.obj");
	Model trainModel(trainObjFileName, false);

	glm::vec3 cameraOffset(0.0f, 2.0f, 2.0f); // Camera este deasupra și în spatele trenului

	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//lightPos.x = 2.5 * cos(glfwGetTime());
		//lightPos.z = 2.5 * sin(glfwGetTime());

		cubePos.x = 10 * sin(glfwGetTime());
		cubePos.z = 10 * cos(glfwGetTime());

		trainPos.z += 0.01;

		lightingShader.use();
		lightingShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", cubePos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());

		lightingShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.setMat4("view", pCamera->GetViewMatrix());

		// Flying Cube
		//glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(0.1f));
		//model = glm::translate(model, cubePos);
		//flyingCubeModel.SetRootTransf(model);
		//lightingShader.setMat4("model", model); //asta era comentata de la profu
		//flyingCubeModel.Draw(lightingShader);

		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingWithTextureShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingWithTextureShader.SetVec3("lightPos", cubePos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);


		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());

		//Pirat
		// 
		//glm::mat4 piratModel = glm::scale(glm::mat4(1.0), glm::vec3(1.f));
		//lightingWithTextureShader.setMat4("model", piratModel);
		//piratObjModel.Draw(lightingWithTextureShader);

		// Draw Grass
		glm::mat4 GrassLawnModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(10000.0f, 1.0, 10000.0));
		lightingWithTextureShader.setMat4("model", GrassLawnModelMatrix);
		GrassLawnModel.Draw(lightingWithTextureShader);

		// TRAIN

		glm::mat4 trainModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0f));
		//trainModelMatrix = glm::translate(model, cubePos);
		trainModelMatrix = glm::rotate(trainModelMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //schimbarea rotatiei
		trainModelMatrix = glm::translate(trainModelMatrix, trainPos);
		lightingWithTextureShader.setMat4("model", trainModelMatrix);
		trainModel.Draw(lightingWithTextureShader);

		if (FPS)
		{
			pCamera->setPosition(cameraOffset - trainPos);
		}

		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();

	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}
