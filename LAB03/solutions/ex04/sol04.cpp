#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_inverse.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "../../camera.h"
#include "../../shader.h"
#include "../object.h"


const int width = 1000;
const int height = 1000;


GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);


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

Camera camera(glm::vec3(1.0, 0.0, -6.0),glm::vec3(0.0,1.0,0.0), 90.0);


int main(int argc, char* argv[])
{
	std::cout << "Welcome to exercice 4: " << std::endl;
	std::cout << "Light - Diffuse Phong\n"
		"Implement Phong shading on a sphere.\n"
		"You need to :\n"
		"	- compare the result with Gouraud, the light must be placed carefully to see the differences \n"
		"	- find what is the difference between the sphere_coarse.obj and sphere_smooth.obj in the object directory?\n"
		"	- compare Gouraud and Phong - rough idea of how many computations are needed for both ? \n";

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
	GLFWwindow* window = glfwCreateWindow(width, height, "Solution 04", nullptr, nullptr);
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

	const std::string sourceV = "#version 330 core\n"
		"in vec3 position; \n"
		"in vec2 tex_coord; \n"
		"in vec3 normal; \n"

		"out vec3 v_normal; \n"
		"out vec3 v_frag_coord; \n"

		"uniform mat4 M; \n"
		"uniform mat4 itM; \n"
		"uniform mat4 V; \n"
		"uniform mat4 P; \n"
		

		" void main(){ \n"
		"vec4 frag_coord = M*vec4(position, 1.0);"
		"gl_Position = P*V*frag_coord;\n"
		//transform correctly the normals then send them to the fragment shader
		"v_normal = vec3(itM * vec4(normal, 1.0)); \n"
		"v_frag_coord = frag_coord.xyz; \n"
		"}\n"; 
	const std::string sourceF = "#version 330 core\n"
		"out vec4 FragColor;"
		"precision mediump float; \n"
		"in vec3 v_normal; \n"
		"in vec3 v_frag_coord; \n"

		"uniform vec3 u_light_pos; \n"

		"void main() { \n"
		//2. compute the ligth using the interpolated normal
		"vec3 L = normalize(u_light_pos - v_frag_coord); \n"
		"float diffusion = max(0.0, dot(v_normal, L)); \n"
		"vec3 color = vec3(diffusion); \n"
		"FragColor = vec4(color, 1.0); \n"
		"} \n";

	
	Shader shader(sourceV, sourceF);


	char path1[] = "../../../../LAB03/objects/sphere_smooth.obj";
	
	Object planet(path1);
	planet.makeObject(shader);
	planet.model = glm::translate(planet.model, glm::vec3(1.0, 0.0, 0.0));
	planet.model = glm::scale(planet.model, glm::vec3(1.5, 1.5, 1.5));

	Object moon1(path1);
	moon1.makeObject(shader);
	moon1.model = glm::translate(moon1.model, glm::vec3(1.0, 0.0, -3.0));
	moon1.model = glm::scale(moon1.model, glm::vec3(0.2, 0.2, 0.2));	


	const glm::vec3 light_pos = glm::vec3(0.5, 5.0, -0.7);
	

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


	//Rendering

	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//2. Use the shader Class to send the uniform
		shader.use();

		shader.setMatrix4("M", planet.model);
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_light_pos", light_pos);

		//planet rotation around itself
		planet.model = glm::rotate(planet.model, glm::radians((float)(4.0f)), glm::vec3(0.0, 1.0, 0.0));

		glm::mat4 itM = glm::inverseTranspose(planet.model);
		shader.setMatrix4("itM", itM);
		planet.draw();
		
		//moon rotation around the planet
		moon1.model = glm::translate(moon1.model, glm::vec3(1.0, 0.0, 9.0));
		moon1.model = glm::rotate(moon1.model, glm::radians((float)(3.0f)), glm::vec3(0.0, 1.0, 0.0));
		moon1.model = glm::translate(moon1.model, glm::vec3(-1.0, 0.0, -9.0));

		shader.setMatrix4("M", moon1.model);
		shader.setMatrix4("itM", glm::inverseTranspose(moon1.model));
		moon1.draw();


		fps(now);
		glfwSwapBuffers(window);
	}

	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


void processInput(GLFWwindow* window) {
	//3. Use the cameras class to change the parameters of the camera
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
		camera.ProcessKeyboardRotation(1, 0.0,1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(-1, 0.0,1);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.ProcessKeyboardRotation(0.0, -1.0, 1);


}
