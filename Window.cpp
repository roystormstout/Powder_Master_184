#include "Window.h"
#define NONE 0
#define ROTATE 1
#define ZOOM 2
#define TRANSLATE 3

#define LEFT 1
#define RIGHT 2

const char* window_title = "184/284A Final Project";

Particles* pe;
Fluid* fluid;
Scene* Window::scene = new Scene();

void Scene::initialize_objects()
{
	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();
	particleShader = new Shader(PART_VERT_SHADER_PATH, PART_FRAG_SHADER_PATH);

	glm::mat4 toWorld(1.0f);

	GLuint particleTexture = loadTexture("../resources/flame.png");
	pe = new Particles(particleTexture, particleShader, { 0,0,0 });
	fluid = new Fluid(width, height);
	pe->bind_fluid(fluid);

	//TODO: remove debug
	//fluid->debug();
	fluid->update(0.1);
	//fluid->debug();
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Scene::clean_up()
{
	delete(camera);

	delete(particleShader);

}

GLFWwindow* Scene::create_window(int width, int height)
{
	// Initialize GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return NULL;
	}

	// 4x antialiasing
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__ // Because Apple hates comforming to standards
	// Ensure that minimum OpenGL version is 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Enable forward compatibility and allow a modern OpenGL context
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Create the GLFW window
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);

	// Check if the window could not be created
	if (!window)
	{
		fprintf(stderr, "Failed to open GLFW window.\n");
		fprintf(stderr, "Either GLFW is not installed or your graphics card does not support modern OpenGL.\n");
		glfwTerminate();
		return NULL;
	}

	// Make the context of the window
	glfwMakeContextCurrent(window);

	// Set swap interval to 1
	glfwSwapInterval(1);

	// Get the width and height of the framebuffer to properly resize the window
	glfwGetFramebufferSize(window, &width, &height);
	// Call the resize callback to make sure things get drawn immediately
	Scene::width = width;
	Scene::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	return window;
}

void Scene::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Scene::width = width;
	Scene::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		camera->SetAspect((float)width / (float)height);
	}
}

void Scene::idle_callback()
{
	camera->Update();
}

void Scene::display_callback(GLFWwindow* window)
{
	auto vpMatrix = camera->GetViewProjectMtx();

	glClearColor(0,0,0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Clear the color and depth buffers
	
	pe->draw();
	//fluid->update(0.01);
	// Use the shader of programID
	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void Scene::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Check for a key press
	if (action == GLFW_PRESS)
	{
		// Check if escape was pressed
		if (key == GLFW_KEY_ESCAPE)
		{
			// Close the window. This causes the program to also terminate.
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}
}

void Scene::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{

		// player moving
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		//printf("Cursor Position at %f: %f \n", xpos, ypos);
		glm::vec3 new_dest = viewToWorldCoordTransform(xpos, ypos);
		pe->update(new_dest);
	}
}

// SCREEN SPACE: mouse_x and mouse_y are screen space
glm::vec3 Scene::viewToWorldCoordTransform(int mouse_x, int mouse_y) {
	// NORMALISED DEVICE SPACE
	double x = 2.0 * mouse_x / width - 1;
	double y = 2.0 * mouse_y / height - 1;

	// HOMOGENEOUS SPACE
	double depth = camera->GetDepth();
	glm::vec4 screenPos = glm::vec4(x, -y, depth, 1.0f);

	// Projection/Eye Space
	glm::mat4 ProjectView = camera->GetViewProjectMtx();
	glm::mat4 viewProjectionInverse = inverse(ProjectView);

	glm::vec4 worldPos = viewProjectionInverse * screenPos;
	//printf("world pos map to: %f %f %f\n", worldPos.x, worldPos.y, worldPos.z);
	glm::vec3 realPos = glm::vec3(worldPos.x / worldPos.w, worldPos.y / worldPos.w, worldPos.z / worldPos.w);


	glm::vec3 cam_pos = camera->cam_pos;
	glm::vec3 dir = glm::normalize(realPos - cam_pos);
	float n = -cam_pos.z / dir.z;
	realPos.x = cam_pos.x + n * dir.x;
	realPos.y = cam_pos.y + n * dir.y;
	realPos.z = 0;

	printf("world pos remap to: %f %f %f\n", realPos.x, realPos.y, realPos.z);

	return realPos;
}
void Scene::cursor_movement_callback(GLFWwindow* window, double x, double y)
{
	// 
	// Handle any necessary mouse movements
	//
	//glm::vec3 direction;
	//float rot_angle;
	//glm::vec3 curPoint;

	//mousePoint = { x, y };
	//if (movement == ROTATE) {
	//	glm::vec2 point = glm::vec2(x, y);
	//	curPoint = trackBallMapping(point); // Map the mouse position to a logical
	//										// sphere location.
	//	direction = curPoint - lastPoint;
	//	float velocity = glm::length(direction);
	//	if (velocity > 0.01) // If little movement - do nothing.
	//	{
	//		//get axis to rotate around
	//		glm::vec3 rotAxis;
	//		rotAxis = glm::cross(lastPoint, curPoint);
	//		rot_angle = velocity * m_ROTSCALE;
	//		float off = 0.3f;
	//		cam_pos = glm::vec3(glm::rotate(glm::mat4(1.0f), rot_angle / 180.0f * glm::pi<float>(), rotAxis) * glm::vec4(cam_pos,1.0f));
	//		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	//	}
	//}
	//lastPoint = curPoint;
}

//resolved
void Scene::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	
}