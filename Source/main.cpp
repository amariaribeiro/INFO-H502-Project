#include<iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_inverse.hpp>

#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "camera.h"
#include "shader.h"
#include "object.h"
#include "shaderInput.h"
#include "lightInput.h"


const int width = 1000;
const int height = 1000;

ShaderInput shaderInput;
LightInput lightInput;

GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);
void loadCubemapFace(const char* file, const GLenum& targetCube);

void defineTexture(GLuint& texture, const char* path);


#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

Camera camera(glm::vec3(1.0, 0.0, -6.0), glm::vec3(0.0, 1.0, 0.0), 90.0);


int main(int argc, char* argv[])
{
	//Boilerplate
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif


	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Project", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);

	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glEnable(GL_DEPTH_TEST);

#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

	//Create and load the textures
	GLuint earth_t;
	defineTexture(earth_t, "../../../../Source/textures/earth.jpg");

	GLuint moon_t;
	defineTexture(moon_t, "../../../../Source/textures/moon.jpg");


	//Sphere objects path
	char path1[] = "../../../../Source/objects/sphere_smooth.obj";

	//path bunny
	char path2[] = "../../../../Source/objects/bunny_small.obj";

	Shader earthShader = Shader(shaderInput.v_earth, shaderInput.f_earth);

	Object moon1(path1);
	moon1.makeObject(earthShader);
	moon1.model = glm::translate(moon1.model, glm::vec3(1.0, 0.0, -3.0));
	moon1.model = glm::scale(moon1.model, glm::vec3(0.2, 0.2, 0.2));

	Object planet(path1);
	planet.makeObject(earthShader);
	planet.model = glm::translate(planet.model, glm::vec3(1.0, 0.0, 0.0));
	planet.model = glm::scale(planet.model, glm::vec3(1.5, 1.5, 1.5));

	//Reflection
	Shader reflShader = Shader(lightInput.reflV, lightInput.reflF);

	Object alien(path2);
	alien.makeObject(reflShader);
	alien.model = glm::translate(alien.model, glm::vec3(0.0, 1.0, - 2.5));
	alien.model = glm::scale(alien.model, glm::vec3(0.1, 0.1, 0.1));

	//Refraction
	Shader refrShader = Shader(lightInput.refrV, lightInput.refrF);

	Object alien2(path1);
	alien2.makeObject(refrShader);
	alien2.model = glm::translate(alien2.model, glm::vec3(2.0, -1.0, -2.5));
	alien2.model = glm::scale(alien2.model, glm::vec3(0.1, 0.1, 0.1));

	

	//CubeMap

	Shader cubeMapShader = Shader(shaderInput.sourceVCubeMap, shaderInput.sourceFCubeMap);

	char pathCube[] = PATH_TO_OBJECTS "/cube.obj";
	Object cubeMap(pathCube);
	cubeMap.makeObject(cubeMapShader);

	GLuint cubeMapTexture;
	glGenTextures(1, &cubeMapTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(false);

	std::string pathToCubeMap = PATH_TO_TEXTURE "/cubemaps/space/";

	std::map<std::string, GLenum> facesToLoad = {
		{pathToCubeMap + "space1.png",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{pathToCubeMap + "space5.png",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{pathToCubeMap + "space2.png",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{pathToCubeMap + "space3.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{pathToCubeMap + "space6.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{pathToCubeMap + "space4.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};
	//load the six faces
	for (std::pair<std::string, GLenum> pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
	}


	const glm::vec3 light_pos = glm::vec3(-5.0, 0.0, -1.5);


	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
		}
	};

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	glm::vec3 materialColour = glm::vec3(0.0, 1.0, 0.0);

	// light constants strengths
	float ambient = 0.2;
	float diffuse = 1.0;
	float specular = 0.9;

	//Rendering

	//defining objects attributes

	refrShader.use();
	refrShader.setFloat("refractionIndice", 1.52);

	earthShader.use();

	earthShader.setFloat("shininess", 32.0f);
	earthShader.setFloat("light.ambient_strength", ambient);
	earthShader.setFloat("light.diffuse_strength", diffuse);
	earthShader.setFloat("light.specular_strength", specular);
	earthShader.setFloat("light.constant", 0.5);
	earthShader.setFloat("light.linear", 0.40);
	earthShader.setFloat("light.quadratic", 0.03);

	


	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		earthShader.use();

		earthShader.setMatrix4("M", planet.model);
		earthShader.setMatrix4("V", view);
		earthShader.setMatrix4("P", perspective);
		earthShader.setVector3f("u_view_pos", camera.Position);
		//earthShader.setVector3f("u_light_pos", light_pos);
		earthShader.setVector3f("light.light_pos", light_pos);

		glm::mat4 itM = glm::inverseTranspose(planet.model);
		earthShader.setMatrix4("itM", itM);

		//earth rotation around itself
		planet.model = glm::rotate(planet.model, glm::radians((float)(0.5f)), glm::vec3(0.0, 1.0, 0.0));

		//earth texture
		glBindTexture(GL_TEXTURE_2D, earth_t);

		planet.draw();

		earthShader.use();

		earthShader.setMatrix4("M", moon1.model);
		earthShader.setMatrix4("V", view);
		earthShader.setMatrix4("P", perspective);
		earthShader.setVector3f("u_light_pos", light_pos);

		earthShader.setMatrix4("M", moon1.model);
		earthShader.setMatrix4("itM", glm::inverseTranspose(moon1.model));

		//moon rotation around the earth
		moon1.model = glm::translate(moon1.model, glm::vec3(1.0, 0.0, 10.0));
		moon1.model = glm::rotate(moon1.model, glm::radians((float)(3.0f)), glm::vec3(0.5, 1.0, 0.0));
		moon1.model = glm::translate(moon1.model, glm::vec3(-1.0, 0.0, -10.0));

		//moon texture
		glBindTexture(GL_TEXTURE_2D, moon_t);

		glDepthFunc(GL_LEQUAL);
		moon1.draw();

		//reflective alien

		reflShader.use();

		reflShader.setMatrix4("M", alien.model);
		reflShader.setMatrix4("V", view);
		reflShader.setMatrix4("P", perspective);

		reflShader.setMatrix4("M", alien.model);
		reflShader.setMatrix4("itM", glm::inverseTranspose(alien.model));

		alien.model = glm::rotate(alien.model, glm::radians((float)(3.0f)), glm::vec3(1.0, 0.0, 1.0));


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		cubeMapShader.setInteger("cubemapTexture", 0);

		alien.draw();

		//refractive

		refrShader.use();

		refrShader.setMatrix4("M", alien2.model);
		refrShader.setMatrix4("V", view);
		refrShader.setMatrix4("P", perspective);

		refrShader.setMatrix4("M", alien2.model);
		refrShader.setMatrix4("itM", glm::inverseTranspose(alien2.model));

		alien2.model = glm::translate(alien2.model, glm::vec3(1.0, 0.0, 1.0));
		alien2.model = glm::rotate(alien2.model, glm::radians((float)(2.0f)), glm::vec3(0.0, -1.0, 0.0));
		alien2.model = glm::translate(alien2.model, glm::vec3(-1.0, 0.0, -1.0));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		cubeMapShader.setInteger("cubemapTexture", 0);
		
		glDepthFunc(GL_LEQUAL);
		alien2.draw();

		cubeMapShader.use();
		cubeMapShader.setMatrix4("V", view);
		cubeMapShader.setMatrix4("P", perspective);
		cubeMapShader.setInteger("cubemapTexture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		cubeMap.draw();
		glDepthFunc(GL_LESS);


		fps(now);

		glfwSwapBuffers(window);

	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void loadCubemapFace(const char* path, const GLenum& targetFace)
{
	int imWidth, imHeight, imNrChannels;
	unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{

		glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		//glGenerateMipmap(targetFace);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_image_free(data);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(LEFT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(RIGHT, 0.1);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboardMovement(BACKWARD, 0.1);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(-1, 0.0, 1);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, -1.0, 1);


}

void defineTexture(GLuint& texture, const char* path) {
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	stbi_image_free(data);
}