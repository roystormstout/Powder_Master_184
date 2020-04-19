#include "Window.h"


#define NONE 0
#define ROTATE 1
#define ZOOM 2
#define TRANSLATE 3

#define LEFT 1
#define RIGHT 2

const char* window_title = "184/284A Final Project";
GLint shaderProgram;

int obj_num,mouse_con;

// On some systems you need to change this to the absolute path

#define PART_VERT_SHADER_PATH "../Particle.vert"
#define PART_FRAG_SHADER_PATH "../Particle.frag"

GLuint particle_program;
const double m_ROTSCALE = 90.0f;
const double m_TRANSCALE = 2.0f;
const double m_ZOOMSCALE = 0.5f;

Particles* pe;

bool debug, tooning, dof;
// Default camera parameters
//glm::vec3 Window::cam_pos(55.0f, 10.0f, 107.0f);		// e  | Position of camera
//glm::vec3 Window::cam_look_at(0, 0.0f, 0.0f);	// d  | This is where the camera looks at
glm::vec3 Window::cam_pos = { 0.0f, 0.0f, -30.0f };		// e  | Position of camera
glm::vec3 Window::cam_look_at = { 0.0f, 0.0f, 0.0f };	// d  | This is where the camera looks at
glm::vec3 Window::cam_up(0.0f, 1.0f, 0.0f);			// up | What orientation "up" is

int Window::width;
int Window::height;

glm::mat4 Window::P;
glm::mat4 Window::V;

glm::vec2 Window::mousePoint;
glm::vec3 Window::lastPoint;
int movement(NONE);


int counter = 0;
void Window::initialize_objects()
{

    particle_program = LoadShaders("../Particle.vert", "../Particle.frag");

	glm::mat4 toWorld(1.0f);

	debug = false;
	pe = new Particles(particle_program);
	tooning = true;
}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Window::clean_up()
{
	//delete(cube);
	//glDeleteProgram(shaderProgram);

}

GLFWwindow* Window::create_window(int width, int height)
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
	Window::resize_callback(window, width, height);

	return window;
}

void Window::resize_callback(GLFWwindow* window, int width, int height)
{
#ifdef __APPLE__
	glfwGetFramebufferSize(window, &width, &height); // In case your Mac has a retina display
#endif
	Window::width = width;
	Window::height = height;
	// Set the viewport size. This is the only matrix that OpenGL maintains for us in modern OpenGL!
	glViewport(0, 0, width, height);

	if (height > 0)
	{
		P = glm::perspective(45.0f, (float)width / (float)height, 0.1f, 2000.0f);
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
}

void Window::idle_callback()
{
}

void Window::display_callback(GLFWwindow* window)
{
	V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	glClearColor(0,0,0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Clear the color and depth buffers
	glUseProgram(particle_program);
	pe->draw(particle_program);
	// Use the shader of programID
	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	// Swap buffers
	glfwSwapBuffers(window);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
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

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	glm::vec2 point = glm::vec2(mousePoint);

	if (action == GLFW_PRESS){
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			movement = ROTATE;
			// Map the mouse position to a logical sphere location.
			// Keep it in the class variable lastPoint.
			lastPoint = trackBallMapping(point);
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			movement = TRANSLATE;
			// Map the mouse position to a logical sphere location.
			// Keep it in the class variable lastPoint.
			lastPoint = trackBallMapping(point);
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
			movement = ZOOM;
			// Map the mouse position to a logical sphere location.
			// Keep it in the class variable lastPoint.
			lastPoint = trackBallMapping(point);
		}
	}
	if (action == GLFW_RELEASE) {
		movement = NONE;
	}
}

glm::vec3 Window::trackBallMapping(glm::vec2 point)
{
	glm::vec3 v;
	float d;
	v.x = (2.0*point.x - width) / width;
	v.y = (height - 2.0*point.y) / height;
	v.z = 0.0;
	d = glm::length(v);
	d = (d<1.0) ? d : 1.0;
	v.z = sqrtf(1.001 - d * d);
	v = glm::normalize(v); // Still need to normalize, since we only capped d, not v.
	return v;
}

void Window::cursor_movement_callback(GLFWwindow* window, double x, double y)
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
void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	/*if (mouse_con == 0) {
		glm::vec3 z_dir = cam_look_at - cam_pos;
			cam_pos -= ((float)-yoffset * glm::normalize(z_dir));
		V = glm::lookAt(cam_pos, cam_look_at, cam_up);
	}
	else {
		glm::vec3 z_dir =  -bull->light_dir;
		bull->light_dir -= ((float)(m_ZOOMSCALE * -yoffset) * glm::normalize(z_dir));
	}*/
}