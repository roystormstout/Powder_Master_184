#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include "shader.h"
#include "Particle.h"
#include "Camera.h"
#include "Box2D.h"
#define PART_VERT_SHADER_PATH "../shaders/Particle.vert"
#define PART_FRAG_SHADER_PATH "../shaders/Particle.frag"

#define BOX_VERT_SHADER_PATH "../shaders/BoxShader.vert"
#define BOX_FRAG_SHADER_PATH "../shaders/BoxShader.frag"

class Scene
{
public:
	 Camera* camera;
	 int width;
	 int height;
	 Shader * particleShader;
	 Shader* boxShader;
	 Box2D* container;
	 glm::vec3 lastPoint;
	 glm::vec2 mousePoint;
	 void initialize_objects();
	 void clean_up();
	 GLFWwindow* create_window(int width, int height);
	 void resize_callback(GLFWwindow* window, int width, int height);
	 void idle_callback();
	 void display_callback(GLFWwindow*);
	 void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	 void cursor_movement_callback(GLFWwindow* window, double x, double y);
	 void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	 void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	 glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y);
};

class Window
{
public:
	static Scene* scene;
	static void initialize_objects() { scene->initialize_objects(); };
	static void clean_up() { scene->clean_up(); };
	static GLFWwindow* create_window(int width, int height) { return scene->create_window(width, height); };
	static void resize_callback(GLFWwindow* window, int width, int height) { return scene->resize_callback(window, width, height); };
	static void idle_callback() { return scene->idle_callback(); };
	static void display_callback(GLFWwindow* window) { return scene->display_callback(window); };
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) { return scene->display_callback(window); };
	static void cursor_movement_callback(GLFWwindow* window, double x, double y) { return scene->cursor_movement_callback(window,x,y); };
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) { return scene->mouse_button_callback(window, button, action, mods); };
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { return scene->scroll_callback(window, xoffset, yoffset); };
	static glm::vec3 viewToWorldCoordTransform(int mouse_x, int mouse_y) { return scene->viewToWorldCoordTransform(mouse_x, mouse_y); };
};
#endif
