#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <random>
#include <thread>
#include <atomic>

#include "Shader.h"

// settings
const unsigned int kScrWidth = 800;
const unsigned int kScrHeight = 600;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void LogicLoop(
	std::atomic<float>& x_pos,
	std::atomic<float>& y_pos,
	std::atomic<float>& x_vel,
	std::atomic<float>& y_vel,
	Shader& shader);

std::atomic<bool> running(true);
std::atomic<bool> collided_flag(false);

// TODO: implement multithreaded approach to rendering/logic
int main()
{
	// GLFW and GLAD setup
	// -------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
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
	float vertices[12] = {
		0.2f,  0.2f, 0.0f,   // top right
		0.2f, -0.2f, 0.0f,   // bottom right
	   -0.2f, -0.2f, 0.0f,   // bottom left
	   -0.2f,  0.2f, 0.0f,   // top left
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	std::mt19937 rng;
	std::uniform_real_distribution<> vel_dist(0.3, 0.5), RGB_dist(0, 1);

	// shared state variables
	std::atomic<float> x_pos = 0.0f, y_pos = 0.0f;
	std::atomic<float> x_vel = vel_dist(rng), y_vel = vel_dist(rng);

	// Launch rendering loop in a separate thread
	std::thread logic_thread(
		LogicLoop,
		std::ref(x_pos),
		std::ref(y_pos),
		std::ref(x_vel),
		std::ref(y_vel),
		std::ref(shader)
	);

	// rendering loop
	// ----------
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();
		glUniform2f(glGetUniformLocation(shader.ID, "velocity"), x_pos.load(), y_pos.load());
		if (collided_flag.load())
		{
			float red = RGB_dist(rng);
			float green = RGB_dist(rng);
			float blue = RGB_dist(rng);
			glUniform3f(glGetUniformLocation(shader.ID, "u_color"), red, green, blue);
			collided_flag.store(false);
		}

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// swap buffers and process events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// signal the logic thread to stop
	running = false;
	
	// sync threads
	logic_thread.join();

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

void LogicLoop(
	std::atomic<float>& x_pos, 
	std::atomic<float>& y_pos,
    std::atomic<float>& x_vel, 
	std::atomic<float>& y_vel,
	Shader& shader)
{
	float prev_time = glfwGetTime();

	while (running)
	{
		float curr_time = glfwGetTime();
		float dt = curr_time - prev_time;
		prev_time = curr_time;

		// update position
		x_pos.fetch_add(x_vel * dt);
		y_pos.fetch_add(y_vel * dt);

		bool collided = false;
		float curr_x = x_pos.load();
		float curr_y = y_pos.load();

		if (curr_x - 0.2f <= -1.0f || curr_x + 0.2f >= 1.0f)
		{
			x_vel = -x_vel;
			collided = true;
		}
		if (curr_y - 0.2f <= -1.0f || curr_y + 0.2f >= 1.0f)
		{
			y_vel = -y_vel;
			collided = true;
		}

		if (collided)
			collided_flag.store(true);
	}
}
