#include<iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_inverse.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "camera.h"
#include "shader.h"
#include "object.h"


const int width = 1000;
const int height = 1000;


GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);

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


	// earth shader

	const std::string v_earth = "#version 330 core\n"
		"in vec3 position; \n"
		"in vec2 tex_coord; \n"
		"in vec3 normal; \n"

		"out vec3 v_normal; \n"
		"out vec3 v_frag_coord; \n"
		"out vec2 v_tex; \n"


		"uniform mat4 M; \n"
		"uniform mat4 itM; \n"
		"uniform mat4 V; \n"
		"uniform mat4 P; \n"

		" void main(){ \n"
		"vec4 frag_coord = M*vec4(position, 1.0);"
		"gl_Position = P*V*frag_coord;\n"
		"v_normal = vec3(itM * vec4(normal, 1.0)); \n"
		"v_frag_coord = frag_coord.xyz; \n"
		"\n"
		"v_tex = tex_coord; \n"
		"}\n";
	const std::string f_earth = "#version 330 core\n"
		"out vec4 FragColor;"
		"precision mediump float; \n"

		"in vec3 v_normal; \n"
		"in vec3 v_frag_coord; \n"
		"in vec2 v_tex; \n"

		"uniform sampler2D texture; \n"
		"uniform vec3 materialColour; \n"

		"uniform vec3 u_view_pos; \n"

		//for the light equation

		"struct Light{\n"
		"vec3 light_pos; \n"
		"float ambient_strength; \n"
		"float diffuse_strength; \n"
		"float specular_strength; \n"
		//attenuation factor
		"float constant;\n"
		"float linear;\n"
		"float quadratic;\n"
		"};\n"
		"uniform Light light;"

		"uniform float shininess; \n"

		"float specularCalculation(vec3 N, vec3 L, vec3 V ){ \n"
		"vec3 R = reflect (-L,N);  \n " //reflect (-L,N) is  equivalent to //max (2 * dot(N,L) * N - L , 0.0) ;
		"float cosTheta = dot(R , V); \n"
		"float spec = pow(max(cosTheta,0.0), 32.0); \n"
		"return light.specular_strength * spec;\n"
		"}\n"


		"void main() { \n"
		//computing light components
		"vec3 N = normalize(v_normal);\n"
		"vec3 L = normalize(light.light_pos - v_frag_coord) ; \n"
		"vec3 V = normalize(u_view_pos - v_frag_coord); \n"
		"float specular = specularCalculation(N, L, V); \n"
		"float diffuse = light.diffuse_strength * max(dot(N,L),0.0);\n"
		"float distance = length(light.light_pos - v_frag_coord);"
		"float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);"
		"float light = light.ambient_strength + attenuation * (diffuse + specular); \n"

		//applying light to object texture
		"FragColor = texture(texture, v_tex) * vec4(light); \n"
		"} \n";



	//Create and load the textures
	GLuint earth_t;
	defineTexture(earth_t, "../../../../Source/textures/earth.jpg");

	GLuint moon_t;
	defineTexture(moon_t, "../../../../Source/textures/moon.jpg");


	//Sphere objects path
	char path1[] = "../../../../Source/objects/sphere_smooth.obj";

	Shader earthShader = Shader(v_earth, f_earth);

	Object moon1(path1);
	moon1.makeObject(earthShader);
	moon1.model = glm::translate(moon1.model, glm::vec3(1.0, 0.0, -3.0));
	moon1.model = glm::scale(moon1.model, glm::vec3(0.2, 0.2, 0.2));

	Object planet(path1);
	planet.makeObject(earthShader);
	planet.model = glm::translate(planet.model, glm::vec3(1.0, 0.0, 0.0));
	planet.model = glm::scale(planet.model, glm::vec3(1.5, 1.5, 1.5));


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