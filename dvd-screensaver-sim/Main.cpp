#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <random>
#include <thread>
#include <atomic>

#include "Shader.h"
#include "stb_image.h"

// settings
const unsigned int kScrWidth = 800;
const unsigned int kScrHeight = 600;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void LogicLoop(
	std::atomic<float>& x_pos,
	std::atomic<float>& y_pos,
	std::atomic<float>& x_vel,
	std::atomic<float>& y_vel);

std::atomic<bool> running(true);
std::atomic<bool> collided_flag(false);
std::atomic<bool> corner_flag(false);

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
	float vertices[32] = {
		// positions           //colors               // texture coords
		0.2f,  0.2f, 0.0f,     1.0f, 0.0f, 0.0f,      1.0f, 1.0f,             // top right
		0.2f, -0.2f, 0.0f,     0.0f, 1.0f, 0.0f,      1.0f, 0.0f,             // bottom right
	   -0.2f, -0.2f, 0.0f,     0.0f, 0.0f, 1.0f,      0.0f, 0.0f,             // bottom left
	   -0.2f,  0.2f, 0.0f,     0.0f, 0.0f, 1.0f,      0.0f, 1.0f              // top left
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and generate texture and mipmaps
	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// texture wrapping/filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, num_channels;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("./dvd_logo_no_bg.png", &width, &height, &num_channels, 0);
	
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else std::cout << "Failed to load texture" << std::endl;

	// free image memory

	stbi_image_free(data);

	std::mt19937 rng;
	std::uniform_real_distribution<> vel_dist(0.3, 0.6), RGB_dist(0.2, 1);

	// shared state variables
	std::atomic<float> x_pos = 0.0f, y_pos = 0.0f;
	std::atomic<float> x_vel = vel_dist(rng), y_vel = vel_dist(rng);

	// Launch rendering loop in a separate thread
	std::thread logic_thread(
		LogicLoop,
		std::ref(x_pos),
		std::ref(y_pos),
		std::ref(x_vel),
		std::ref(y_vel)
	);

	bool collided_once = false;
	// rendering loop
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		shader.Use();

		if (!collided_once)
			glUniform3f(glGetUniformLocation(shader.ID, "u_color"), 1.0f, 1.0f, 1.0f);

		glUniform2f(glGetUniformLocation(shader.ID, "velocity"), x_pos.load(), y_pos.load());
		if (collided_flag.load())
		{
			if (corner_flag.load())
			{
				glUniform3f(glGetUniformLocation(shader.ID, "u_color"), 0.0f, 0.0f, 0.0f);
				corner_flag.store(false);
			}
			else
			{
				float red = RGB_dist(rng);
				float green = RGB_dist(rng);
				float blue = RGB_dist(rng);
				glUniform3f(glGetUniformLocation(shader.ID, "u_color"), red, green, blue);
				collided_flag.store(false);
				collided_once = true;
			}
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
	std::atomic<float>& y_vel)
{
	const float debounce_time = 0.1f;
	float last_collision_time = 0.0f;

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
		bool corner = false;
		float curr_x = x_pos.load();
		float curr_y = y_pos.load();

		bool hit_horizontal = curr_x - 0.2f <= -1.0f || curr_x + 0.2f >= 1.0f;
		bool hit_vertical = curr_y - 0.2f <= -1.0f || curr_y + 0.2f >= 1.0f;
		bool is_in_debounce_range = curr_time - last_collision_time > debounce_time;

		if (is_in_debounce_range)
		{
			if (hit_horizontal)
			{
				x_vel = -x_vel;
				collided = true;
			}
			if (hit_vertical)
			{
				y_vel = -y_vel;
				collided = true;
			}
		}

		if (hit_horizontal && hit_vertical)
			corner = true;

		if (collided)
		{
			last_collision_time = curr_time;
			collided_flag.store(true);
			if (corner)
				corner_flag.store(true);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}
