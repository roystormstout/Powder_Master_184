#include "Window.h"
#include "GUILayout.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

const char* window_title = "184/284A Final Project";

Particles* pe;
Scene* Window::scene = new Scene();
struct nk_context* ctx;
struct nk_colorf bg = { 0.1f,0.1f,0.1f,1.0f };
struct media media;
struct guiStatus gui_status;

void Scene::initialize_objects()
{	camera = new Camera();
	camera->SetAspect(width / height);
	camera->Reset();
	particleShader = new Shader(PART_VERT_SHADER_PATH, PART_FRAG_SHADER_PATH);
	boxShader = new Shader(BOX_VERT_SHADER_PATH, BOX_FRAG_SHADER_PATH);
	glm::mat4 toWorld(1.0f);

	GLuint particleTexture = loadTexture("../resources/spark.png");
	pe = new Particles(particleTexture, particleShader, { 0,0,0 });
	container = new Box2D({ BOX_SIDE_LENGTH / 2,BOX_SIDE_LENGTH / 2,0 }, { -BOX_SIDE_LENGTH / 2,-BOX_SIDE_LENGTH / 2,0 }, boxShader);

}

// Treat this as a destructor function. Delete dynamically allocated memory here.
void Scene::clean_up()
{
	delete(camera);

	delete(particleShader);

}

void Scene::initialize_UI(GLFWwindow* window) {

	ctx = nk_glfw3_init(window, NK_GLFW3_DEFAULT);
	{const void* image; int w, h;
	struct nk_font_config cfg = nk_font_config(0);
	struct nk_font_atlas* atlas;
	nk_glfw3_font_stash_begin(&atlas);
	media.font_14 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 14.0f, &cfg);
	media.font_18 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 18.0f, &cfg);
	media.font_20 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 20.0f, &cfg);
	media.font_22 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 22.0f, &cfg);
	media.font_32 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 32.0f, &cfg);
	media.font_48 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 48.0f, &cfg);
	media.font_64 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 64.0f, &cfg);
	media.font_128 = nk_font_atlas_add_from_file(atlas, "../Nuklear/extra_font/monogram_extended.ttf", 128.0f, &cfg);
	nk_glfw3_font_stash_end();
	}
	glfw.atlas.default_font = media.font_32;
	nk_style_set_font(ctx, &(media.font_32->handle));
	gui_status.curr_parts = 0;
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
	//TODO: ADD PARTICLE PHYSICS UPDATE
	pe->update();
	gui_status.curr_parts = pe->ParticlesCount;
}

void Scene::display_callback(GLFWwindow* window)
{
	auto vpMatrix = camera->GetViewProjectMtx();

	glClearColor(0.3, 0.3, 0.3, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Clear the color and depth buffers
	
	pe->draw();
	container->draw();
	// Use the shader of programID
	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	nk_glfw3_new_frame();
	/* GUI */


	ui_statstics(ctx, &media, gui_status);

	/* ----------------------------------------- */


	nk_glfw3_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
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
		//TODO: CHANGE FUNCTION NAME TO DISTANCE_UPDATE
		pe->move_to(new_dest);
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