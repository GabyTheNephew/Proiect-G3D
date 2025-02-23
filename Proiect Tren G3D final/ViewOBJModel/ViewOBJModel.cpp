// ViewOBJModel.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <Windows.h>
#include <locale>
#include <codecvt>

#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stb_image.h>
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
#include "AudioManager.h"

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "OpenAL32.lib")

// settings
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;

//pus de mine
bool FPS = true;

float trainAcceleration = 1;
glm::vec3 trainRouteStart(0.0f, 0.0f, 0.0f);
glm::vec3 trainRouteEnd(0.0f, 0.0f, 20.0f);
float trainRouteWidth = 10.0f; // Train route's width
float stationWidth = 20.f;

Shader lightingShader;
Shader lightingWithTextureShader;
Shader lampShader;
Shader depthShader;

glm::vec3 lightPos(0.0f, 100.0f, -100.0f);
glm::vec3 cubePos(0.0f, 5.0f, 1.0f);
glm::vec3 trainPos(0.0f, 0.27f, 0.0f);

glm::vec3 railStartPos(0.0f, 0.0f, -100.0f);
glm::vec3 railDirection(0.0f, 0.0f, 1.0f);

//wchar_t buffer[MAX_PATH];
//GetCurrentDirectoryW(MAX_PATH, buffer);

//std::wstring executablePath(buffer);
//std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

//std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
//std::string currentPath = converter.to_bytes(wscurrentPath);


std::string GrassLawnFileName;
Model GrassLawnModel;

std::string trainObjFileName;
Model trainModel;

std::string treeObjFileName;
Model treeModel;

std::string stationObjFileName;
Model stationModel;

std::string railroadObjFileName;
Model railroadModel;

std::string rockObjFileName;
Model rockModel;

std::string cartObjFileName;
Model cartModel;

std::vector<glm::vec3> stationPosition(3);


unsigned int loadCubemap(std::vector<std::string> faces);
unsigned int skyboxVAO, skyboxVBO;
unsigned int cubemapTexture;

AudioManager* audioManager = nullptr;
ALuint trainSound = 0;	
ALuint trainWsitle = 0;

float distanceBetweenGrass = 200.0f; // Distanța între bucăți de teren

std::vector<glm::vec3> grassPositions;
int numGrassPieces;
std::vector<glm::vec3> treePositions;

std::vector<glm::vec3> railPositions(85);

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
	const float zFAR = 5000.f;
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
	delete audioManager;
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

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		if (trainAcceleration < 10)
		{
			trainAcceleration += 0.1;
			std::cout << "Train acceleration: " << trainAcceleration << std::endl;

			if (!audioManager->isPlaying(trainSound)) {
				std::cout << "Attempting to play sound..." << std::endl;
				audioManager->playSound(trainSound);

				// Verifică dacă sunetul chiar se redă
				if (audioManager->isPlaying(trainSound)) {
					std::cout << "Sound is now playing!" << std::endl;
				}
				else {
					std::cout << "Failed to start playing sound!" << std::endl;
				}
			}
			else
			{
				audioManager->setPitch(trainSound, 0.5f + (trainAcceleration / 10.0f));
			}
		}
	}

	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		if (trainAcceleration > 0)
		{
			std::cout << "Train acceleration: " << trainAcceleration << std::endl;
			trainAcceleration -= 0.1;
			if (trainAcceleration <= 0) {
				audioManager->stopSound(trainSound);
			}
			else {
				audioManager->setPitch(trainSound, 0.5f + (trainAcceleration / 10.0f));
			}
		}
		if (trainAcceleration < 0)
		{
			trainAcceleration = 0;
		}
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		audioManager->playSound(trainWsitle);
	}

}



bool IsNearTrainRoute(const glm::vec3& position) {
	if (position.x <= trainRouteWidth && position.x >= -trainRouteWidth)
	{
		return true;
	}
	if (position.x <= stationWidth && position.x >= -stationWidth && position.z <= stationWidth && position.z >= -stationWidth)
	{
		return true;
	}

	return false;
}

std::vector<glm::vec3> GenerateTreePositions(int treeCount, float areaWidth, float areaDepth) {
	std::vector<glm::vec3> treePositions;
	srand(static_cast<unsigned>(time(0))); // Seed the random number generator

	for (int i = 0; i < treeCount; ++i) {
		glm::vec3 position;
		do {
			position.x = ((float)rand() / RAND_MAX) * areaWidth - (areaWidth / 2);
			position.y = 0.0f;
			position.z = -std::abs(((float)rand() / RAND_MAX) * areaDepth - (areaDepth / 2)) + 75.0f;

		} while (IsNearTrainRoute(position));
		treePositions.push_back(position);
	}
	return treePositions;
}

std::vector<glm::vec3> GenerateRockPositions(int rockCount, float spacing) {
	std::vector<glm::vec3> rockPositions;
	float startZ = 200.0f;

	srand(static_cast<unsigned>(time(0)));

	for (int i = 0; i < rockCount; ++i) {
		float side = (i % 2 == 0) ? 1.0f : -1.0f;

		
		float randomX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 4.0f; 
		float randomY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 6.0f; 
		float randomZ = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * 0.7f; 

		glm::vec3 position;
		position.x = (side * 12.0f) + randomX;   
		position.y = startZ + (i * spacing) + randomY;
		position.z = 2.9f + randomZ;

		rockPositions.push_back(position);
	}
	return rockPositions;
}

GLuint CreateDepthMapFBO(GLuint& depthMapTexture, const unsigned int shadowWidth, const unsigned int shadowHeight) {
	GLuint depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// Creați textura pentru hartă de adâncime
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Bordura pentru cazuri unde nu există informație de adâncime
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Atașează textura la framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Depth framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return depthMapFBO;
}

void RenderDepthMap(GLuint& depthMapFBO, Shader& shaderProgram, glm::mat4& lightSpaceMatrix, Camera& camera) {
	glViewport(0, 0, 8192, 8192); // Rezoluția hărții de adâncime
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	shaderProgram.use();

	shaderProgram.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	// Randare modele doar pentru hartă de umbre
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, trainPos);
	shaderProgram.setMat4("model", model);
	trainModel.Draw(shaderProgram);

	model = glm::scale(glm::mat4(1.0f), glm::vec3(10000.0f, 1.0f, 10000.0f));
	shaderProgram.setMat4("model", model);
	GrassLawnModel.Draw(shaderProgram);

	// Randare copaci
	for (const auto& pos : treePositions) {
		glm::mat4 treeModelMatrix = glm::mat4(1.0f);
		treeModelMatrix = glm::translate(treeModelMatrix, pos);
		treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(0.5f));
		// treeModelMatrix = glm::rotate(treeModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		shaderProgram.setMat4("model", treeModelMatrix);
		treeModel.Draw(shaderProgram);
	}

	// Randare teren
	/*glm::mat4 GrassLawnModelMatrix = glm::mat4(1.0f);
	GrassLawnModelMatrix = glm::translate(GrassLawnModelMatrix, -grassPositions[i]);
	GrassLawnModelMatrix = glm::scale(GrassLawnModelMatrix, glm::vec3(10000.0f, 10000.0f, 10000.0f));
	lightingWithTextureShader.setMat4("model", GrassLawnModelMatrix);
	GrassLawnModel.Draw(lightingWithTextureShader);*/

	//RAILROAD
	for (const auto& pos : railPositions)
	{
		/*glm::vec3 segmentPosition = -(railStartPos + i * 18.0f * railDirection);*/
		glm::mat4 railroadModelMatrix = glm::mat4(1.0f);
		railroadModelMatrix = glm::translate(railroadModelMatrix, pos);
		railroadModelMatrix = glm::scale(railroadModelMatrix, glm::vec3(0.42f));
		shaderProgram.setMat4("model", railroadModelMatrix);
		railroadModel.Draw(shaderProgram);
	}

	//Cart
	for (int i = 1; i < 5; i++)
	{
		glm::mat4 cartModelMatrix = glm::mat4(1.0);
		cartModelMatrix = glm::translate(cartModelMatrix, trainPos - glm::vec3(0.0f, 0.0f, 2.0f) + glm::vec3(0.0f, 0.0f, i * 8.3f));
		shaderProgram.setMat4("model", cartModelMatrix);
		cartModel.Draw(shaderProgram);
	}

	//STATION
	for (auto& pos : stationPosition)
	{

		glm::mat4 stationModelMatrix = glm::mat4(1.0f);
		stationModelMatrix = glm::translate(stationModelMatrix, glm::vec3(0.7f, 0.0f, pos.z));
		stationModelMatrix = glm::rotate(stationModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		stationModelMatrix = glm::scale(stationModelMatrix, glm::vec3(0.1f));
		shaderProgram.setMat4("model", stationModelMatrix);
		stationModel.Draw(shaderProgram);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // Revenim la rezoluția normală
}

// load cubemap textures
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
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

	float skyboxVertices[] = {
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
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


	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0, 3.0, 3.0));

	//// FBO pentru umbre
	//unsigned int depthMapFBO;
	//glGenFramebuffers(1, &depthMapFBO);

	//// Textura pentru harta umbrelor
	//unsigned int depthMap;
	//glGenTextures(1, &depthMap);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//// Atașare la framebuffer
	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);




	//std::vector<glm::vec3> treePositions = GenerateTreePositions(200, 200.0f, 180.0f);

	std::vector<glm::vec3> rockPositions = GenerateRockPositions(10, 20.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	lightingShader = Shader((currentPath + "\\Shaders\\PhongLight.vs").c_str(), (currentPath + "\\Shaders\\PhongLight.fs").c_str());
	lightingWithTextureShader = Shader((currentPath + "\\Shaders\\PhongLightWithTexture.vs").c_str(), (currentPath + "\\Shaders\\PhongLightWithTexture.fs").c_str());
	lampShader = Shader((currentPath + "\\Shaders\\Lamp.vs").c_str(), (currentPath + "\\Shaders\\Lamp.fs").c_str());
	depthShader = Shader((currentPath + "\\Shaders\\depth_shader.vs").c_str(),
		(currentPath + "\\Shaders\\depth_shader.fs").c_str());

	
	GrassLawnFileName = (currentPath + "\\Models\\GrassLawn\\grassLawn.obj");
	GrassLawnModel = Model(GrassLawnFileName, false);

	trainObjFileName = (currentPath + "\\Models\\Train\\train.obj");
	trainModel = Model(trainObjFileName, false);

	treeObjFileName = (currentPath + "\\Models\\Tree_v2\\tree_v2.obj");
	treeModel = Model(treeObjFileName, false);

	stationObjFileName = (currentPath + "\\Models\\TrainStation\\Train_Station.obj");
	stationModel = Model(stationObjFileName, false);

	railroadObjFileName = (currentPath + "\\Models\\Railroad\\railroad.obj");
	railroadModel = Model(railroadObjFileName, false);

	rockObjFileName = (currentPath + "\\Models\\Rock\\rock.obj");
	rockModel = Model(rockObjFileName, false);
	
	cartObjFileName = (currentPath + "\\Models\\TrainCart\\Gondola_car.obj");
	cartModel = Model(cartObjFileName, false);

	std::string rotiMiciObjFileName = (currentPath + "\\Models\\RotiMici\\rotimici.obj");
	Model rotiMiciModel(rotiMiciObjFileName, false);

	std::string rotiMariObjFileName = (currentPath + "\\Models\\RotiMari\\rotimari.obj");
	Model rotiMariModel(rotiMariObjFileName, false);



	audioManager = new AudioManager();
	if (!audioManager) {
		std::cout << "Failed to create AudioManager!" << std::endl;
		return -1;
	}
	std::string trainSoundPath = currentPath + "\\Sounds\\train_on_traks.wav";
	trainSound = audioManager->loadSound(trainSoundPath.c_str());
	std::string trainWistlePath = currentPath + "\\Sounds\\wistle.wav";
	trainWsitle = audioManager->loadSound(trainWistlePath.c_str());
	if (trainSound == 0) {
		std::cout << "Failed to load train sound from: " << trainSoundPath << std::endl;
		return -1;
	}
	else {
		std::cout << "Successfully loaded train sound!" << std::endl;
	}
	audioManager->setVolume(trainSound, 0.1f);
	audioManager->setLooping(trainSound, true);
	audioManager->playSound(trainSound);
	

	std::vector<std::string> faces
	{ 
		currentPath + "\\Daylight Box_Pieces\\Daylight Box_Right.bmp",
		currentPath + "\\Daylight Box_Pieces\\Daylight Box_Left.bmp",
		currentPath + "\\Daylight Box_Pieces\\Daylight Box_Top.bmp",
		currentPath + "\\Daylight Box_Pieces\\Daylight Box_Bottom.bmp",
		currentPath + "\\Daylight Box_Pieces\\Daylight Box_Front.bmp",
		currentPath + "\\Daylight Box_Pieces\\Daylight Box_Back.bmp"
	};
	cubemapTexture = loadCubemap(faces);

	// Create skybox shader
	Shader skyboxShader((currentPath + "\\Shaders\\skybox.vs").c_str(), (currentPath + "\\Shaders\\skybox.fs").c_str());
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	glm::vec3 cameraOffset(0.0f, 2.0f, 2.0f); 


	float distanceBetweenGrass = 200.0f; // Distanța între bucăți de teren

	grassPositions = {
	glm::vec3(0.0f, 4.45f, 0.0f),
	glm::vec3(0.0f, 4.45f, distanceBetweenGrass),
	glm::vec3(0.0f, 4.45f, 2 * distanceBetweenGrass)
	};
	numGrassPieces = grassPositions.size();
	treePositions = GenerateTreePositions(200, 200.0f, 1100.0f);

	//std::vector<glm::vec3> railPositions(85);
	for (int i = 0;i < 85;i++)
	{
		railPositions[i] = -(railStartPos + i * 7.0f * railDirection);
	}

	for (int i = 0; i < 3; i++)
	{
		stationPosition[i] = glm::vec3(0.0f, 0.0f, -i * 200.0f);
	}



	//static glm::vec3 lightPos(0.0f, 20.0f, -10.0f); // Poziție fixă
	/*glm::mat4 lightProjection = glm::ortho(-300.0f, 300.0f, -300.0f, 300.0f, 1.0f, 600.0f);
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;*/

	int numberOfGrassMoves = 1;
	int indexOfStation = 0;

	GLuint depthMapFBO, depthMap;
	depthMapFBO = CreateDepthMapFBO(depthMap, 8192, 8192);
	// render loop
	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glm::vec3 lightTarget = trainPos;
		glm::vec3 lightDir = glm::normalize(lightPos - lightTarget);
		//glm::mat4 lightProjection = glm::ortho(-200.0f, 200.0f, -200.0f, 200.0f, 0.1f, 300.0f);
		glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);

		/*glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		if (glm::dot(lightDir, up) > 0.99f) {
		up = glm::vec3(1.0f, 0.0f, 0.0f);
		}

		glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, up);*/
		glm::mat4 lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;

		// Render Depth Map
		RenderDepthMap(depthMapFBO, depthShader, lightSpaceMatrix, *pCamera);



		//glViewport(0, 0, 4096, 4096);
		/*glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);*/



		// glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Revenire la viewport-ul normal
		/*glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/



		//lightPos.x = 2.5 * cos(glfwGetTime());
		//lightPos.z = 2.5 * sin(glfwGetTime());

		cubePos.x = 10 * sin(glfwGetTime());
		cubePos.z = 10 * cos(glfwGetTime());

		trainPos.z -= 0.05f * trainAcceleration;
		lightPos.z = trainPos.z - 200.0f;

		lightingShader.use();
		lightingShader.SetVec3("objectColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());

		lightingShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.setMat4("view", pCamera->GetViewMatrix());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("objectColor", 1.0f, 1.0f, 1.0f);
		lightingWithTextureShader.SetVec3("lightColor", 1.5f, 1.5f, 1.5f);
		lightingWithTextureShader.SetVec3("lightPos", lightPos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.SetVec3("globalAmbient", 0.05f, 0.05f, 0.05f);
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);
		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		lightingWithTextureShader.setInt("shadowMap", 1); // Textura umbrelor
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap); // Leagă textura umbrelor

		// Setarea matricilor

		//// Draw Grass
		//glm::mat4 GrassLawnModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(10000.0f, 10000.0, 10000.0));
		//GrassLawnModelMatrix = glm::translate(GrassLawnModelMatrix, glm::vec3(0.0f, -0.000450f, 0.0f));
		//lightingWithTextureShader.setMat4("model", GrassLawnModelMatrix);
		//GrassLawnModel.Draw(lightingWithTextureShader);


		// TRAIN

		glm::mat4 trainModelMatrix = glm::scale(glm::mat4(1.0), glm::vec3(1.0f));
		trainModelMatrix = glm::translate(trainModelMatrix, trainPos);
		lightingWithTextureShader.setMat4("model", trainModelMatrix);
		trainModel.Draw(lightingWithTextureShader);


		//Wheels

			//Mici
		glm::mat4 rotiMiciModelMatrix = glm::mat4(1.0);
		rotiMiciModelMatrix = glm::translate(rotiMiciModelMatrix, trainPos + glm::vec3(0.0f, 0.30f, -0.45f));
		rotiMiciModelMatrix = glm::rotate(rotiMiciModelMatrix, glm::radians(trainPos.z * 150), glm::vec3(1.0f, 0.0f, 0.0f));
		rotiMiciModelMatrix = glm::scale(rotiMiciModelMatrix, glm::vec3(1.0f));
		lightingWithTextureShader.setMat4("model", rotiMiciModelMatrix);
		rotiMiciModel.Draw(lightingWithTextureShader);

		rotiMiciModelMatrix = glm::mat4(1.0);
		rotiMiciModelMatrix = glm::translate(rotiMiciModelMatrix, trainPos + glm::vec3(0.0f, 0.30f, -1.15f));
		rotiMiciModelMatrix = glm::rotate(rotiMiciModelMatrix, glm::radians(trainPos.z * 150), glm::vec3(1.0f, 0.0f, 0.0f));
		rotiMiciModelMatrix = glm::scale(rotiMiciModelMatrix, glm::vec3(1.0f));
		lightingWithTextureShader.setMat4("model", rotiMiciModelMatrix);
		rotiMiciModel.Draw(lightingWithTextureShader);

			//Mari
		glm::mat4 rotiMariModelMatrix = glm::mat4(1.0);
		rotiMariModelMatrix = glm::translate(rotiMariModelMatrix, trainPos + glm::vec3(0.0f,0.45f,0.85f));
		rotiMariModelMatrix = glm::rotate(rotiMariModelMatrix, glm::radians(trainPos.z * 100), glm::vec3(1.0f, 0.0f, 0.0f));
		rotiMariModelMatrix = glm::scale(rotiMariModelMatrix, glm::vec3(1.0f));
		lightingWithTextureShader.setMat4("model", rotiMariModelMatrix);
		rotiMariModel.Draw(lightingWithTextureShader);

		rotiMariModelMatrix = glm::mat4(1.0);
		rotiMariModelMatrix = glm::translate(rotiMariModelMatrix, trainPos + glm::vec3(0.0f, 0.45f, 1.85f));
		rotiMariModelMatrix = glm::rotate(rotiMariModelMatrix, glm::radians(trainPos.z * 100), glm::vec3(1.0f, 0.0f, 0.0f));
		rotiMariModelMatrix = glm::scale(rotiMariModelMatrix, glm::vec3(1.0f));
		lightingWithTextureShader.setMat4("model", rotiMariModelMatrix);
		rotiMariModel.Draw(lightingWithTextureShader);



		//CART
		for (int i = 1; i < 5; i++)
		{
			lightingWithTextureShader.use();
			glm::mat4 cartModelMatrix = glm::mat4(1.0);
			cartModelMatrix = glm::translate(cartModelMatrix, trainPos - glm::vec3(0.0f, 0.0f, 2.0f) + glm::vec3(0.0f, 0.0f, i * 8.3f));
			lightingWithTextureShader.setMat4("model", cartModelMatrix);
			cartModel.Draw(lightingWithTextureShader);
		}


		// Actualizarea și desenarea bucăților de teren
		for (int i = 0; i < numGrassPieces; ++i) {
			if (-trainPos.z >= grassPositions[i].z + distanceBetweenGrass) {
				// Actualizează poziția bucății de teren
				float shiftDistance = distanceBetweenGrass * numGrassPieces;

				grassPositions[i].z += shiftDistance;

				// Repoziționarea corectă a copacilor și rocilor
				for (auto& pos : treePositions) {
					pos.z -= distanceBetweenGrass;  // Folosim semnul corect pentru a muta în față
				}
				for (auto& pos : rockPositions) {
					pos.z -= shiftDistance;
				}
				for (auto& pos : railPositions)
				{
					pos.z -= distanceBetweenGrass;
				}
				for (auto& pos : stationPosition)
				{
					pos.z -= distanceBetweenGrass;
				}
			}

			// Randare teren
			glm::mat4 GrassLawnModelMatrix = glm::mat4(1.0f);
			GrassLawnModelMatrix = glm::translate(GrassLawnModelMatrix, -grassPositions[i]);
			GrassLawnModelMatrix = glm::scale(GrassLawnModelMatrix, glm::vec3(10000.0f, 10000.0f, 10000.0f));
			lightingWithTextureShader.setMat4("model", GrassLawnModelMatrix);
			GrassLawnModel.Draw(lightingWithTextureShader);
		}

		
		



		////TREES
		//for (const auto& pos : treePositions) {
		//	glm::mat4 treeModelMatrix = glm::mat4(1.0);
		//	treeModelMatrix = glm::translate(treeModelMatrix, pos);
		//	treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(0.5f));
		//	//treeModelMatrix = glm::rotate(treeModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //schimbarea rotatiei
		//	lightingWithTextureShader.setMat4("model", treeModelMatrix);
		//	treeModel.Draw(lightingWithTextureShader);
		//}

		// Randare copaci
		for (const auto& pos : treePositions) {
			lightingWithTextureShader.use();
			glm::mat4 treeModelMatrix = glm::mat4(1.0f);
			treeModelMatrix = glm::translate(treeModelMatrix, pos);
			treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(0.5f));
			// treeModelMatrix = glm::rotate(treeModelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			lightingWithTextureShader.setMat4("model", treeModelMatrix);
			treeModel.Draw(lightingWithTextureShader);
		}

		//RAILROAD
		for (const auto& pos : railPositions) 
		{
			/*glm::vec3 segmentPosition = -(railStartPos + i * 18.0f * railDirection);*/
			glm::mat4 railroadModelMatrix = glm::mat4(1.0f);
			railroadModelMatrix = glm::translate(railroadModelMatrix, pos);
			railroadModelMatrix = glm::scale(railroadModelMatrix, glm::vec3(0.42f));
			lightingWithTextureShader.setMat4("model", railroadModelMatrix);
			railroadModel.Draw(lightingWithTextureShader);
		}




		//STATION
		for (auto& pos : stationPosition)
		{
			lightingWithTextureShader.use();
			glm::mat4 stationModelMatrix = glm::mat4(1.0f);
			stationModelMatrix = glm::translate(stationModelMatrix, glm::vec3(0.7f, 0.0f, pos.z));
			stationModelMatrix = glm::rotate(stationModelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			stationModelMatrix = glm::scale(stationModelMatrix, glm::vec3(0.1f));
			lightingWithTextureShader.setMat4("model", stationModelMatrix);
			stationModel.Draw(lightingWithTextureShader);
		}



		if (FPS)
		{
			pCamera->setPosition(cameraOffset + trainPos + glm::vec3(0.0f, 0.0f, -trainAcceleration * 0.05));
		}


		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix())));
		skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		

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
