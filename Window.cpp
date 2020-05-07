#include "Window.h"
#include "GUILayout.h"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

const char* window_title = "184/284A Final Project";

Particles* pe;
Fluid* fluid;
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
	fluid = new Fluid(width, height);
	pe = new Particles(particleTexture, particleShader, fluid);
	container = new Box2D({ BOX_SIDE_LENGTH / 2,BOX_SIDE_LENGTH / 2,0 }, { -BOX_SIDE_LENGTH / 2,-BOX_SIDE_LENGTH / 2,0 }, boxShader);

	lastFPSTime = glfwGetTime();
	lastSpawnTime = glfwGetTime();
	isSpawning = false;
	framePassed = 0;
	type = water;
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
	media.water[0] = icon_load("../resources/icons/water.png");
	media.water[1] = icon_load("../resources/icons/water_highlighted.png");
	media.rock[0] = icon_load("../resources/icons/rock.png");
	media.rock[1] = icon_load("../resources/icons/rock_highlighted.png");
	gui_status.curr_parts = 0;
	gui_status.type = type;
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

	double currentTime = glfwGetTime();
	framePassed++;
	if (currentTime - lastFPSTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
		gui_status.fps = framePassed;
		if(DEBUG)
			printf("%f ms/frame\n", 1000.0 / double(framePassed));
		framePassed = 0;
		lastFPSTime += 1.0;
	}
	if (type == water && isSpawning && currentTime - lastSpawnTime >= 0.2f) {
		pe->spawn_at(cursorWorldPos, { 0,255,255 }, { 0,-GRAVITY,0 },water);
		lastSpawnTime = currentTime;
	}
	camera->Update();
	pe->update();
	fluid->update(DELTA_TIME);
	gui_status.curr_parts = pe->ParticlesCount;
}

void Scene::display_callback(GLFWwindow* window)
{
	auto vpMatrix = camera->GetViewProjectMtx();

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Clear the color and depth buffers
	
	pe->draw();
	container->draw();
	// Use the shader of programID
	// Gets events, including input such as keyboard and mouse or window resizing
	glfwPollEvents();
	nk_glfw3_new_frame();
	/* GUI */


	main_layout(ctx, &media, width, height, gui_status);

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
		if (key == GLFW_KEY_Q)
		{
			type = water;
			gui_status.type = water;
		}
		if (key == GLFW_KEY_W)
		{
			cout << "W pressed " << endl;
			type = rock;
			gui_status.type = rock;
			isSpawning = false;
		}
	}
}


void Scene::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && type == water)
	{
		isSpawning = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && isSpawning && type == water) {
		isSpawning = false;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !isSpawning && type == rock) {
		pe->spawn_at(cursorWorldPos, { 230,178,134}, { 0,0,0 }, rock);
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

	//printf("world pos remap to: %f %f %f\n", realPos.x, realPos.y, realPos.z);

	return realPos;
}
void Scene::cursor_movement_callback(GLFWwindow* window, double x, double y)
{
	// player moving
	double xpos, ypos;
	//getting cursor position
	glfwGetCursorPos(window, &xpos, &ypos);
	//printf("Cursor Position at %f: %f \n", xpos, ypos);
	cursorWorldPos = viewToWorldCoordTransform(xpos, ypos);
}

//resolved
void Scene::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	
}