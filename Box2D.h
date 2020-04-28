#ifndef _BOX2D_H
#define _BOX2D_H


#include "Particle.h"
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <GL/glut.h>


#pragma once
class Box2D {
	//assume rectangular and axis aligned
public:
	 glm::vec3 max, min; //corners
	 GLuint VBO, VAO, EBO;
	 Shader* shader;
	bool in_box(Particle*); //returns true if particle in container, else it reflects particle and returns false.
	Box2D(glm::vec3, glm::vec3, Shader *);
	void draw();
	Box2D(void) {};
	~Box2D(void) {};
};

#endif