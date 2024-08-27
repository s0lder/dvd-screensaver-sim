#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

// settings
const unsigned int kScrWidth = 800;
const unsigned int kScrHeight = 600;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

int main()
{
	// GLFW and GLAD setup
	// -------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	GLFWwindow* window = glfwCreateWindow(kScrWidth, kScrHeight, "DVD Screensaver Simulator", NULL, NULL);

	// error handling
	const char* description;
	int code = glfwGetError(&description);
	if (code != GLFW_NO_ERROR)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		std::cout << "GLFW error: " << description << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	
	// GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// OpenGL app
	// ----------
	// Shader initialization using the Shader class
	Shader shader("./vertex.glsl", "./fragment.glsl");

	// Vertex data
	float vertices[24] = {
		// positions        //colors
		0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // top right
		0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom right
	   -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // bottom left
	   -0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f    // top left
	};

	unsigned int indices[6] = {
		0, 1, 3,  // first triangle
		1, 2, 3   // second triangle
	};

	// EBO, VBO and VAO
	unsigned int EBO, VBO, VAO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// set and enable vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// graphics loop
	while (!glfwWindowShouldClose(window))
	{
		// TODO: handle input

		// rendering
		shader.Use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// process events and swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// de-allocate all resources
	shader.Delete();
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();

	return 0;
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
