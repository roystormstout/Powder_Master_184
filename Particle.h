#ifndef _PARTICLE_H
#define _PARTICLE_H

#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <cmath>
#include <vector>
#include <list>
#include "glm/ext.hpp"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <math.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <random>
#include "texture.h"
#include "shader.h"

#define DEBUG false
#define TENSILE_INSTABILITY true
#define VISCOSITY_EFFECT false
#define MAX_PARTICLES 40
#define PARTICLE_SIZE 0.5
#define REST_DENSITY 250
#define PI 3.14159265
#define GRAVITY 9.8
#define SOLVER_ITER 3
#define BOX_SIDE_LENGTH 4
#define EPSILON 700
//for tensile instability
#define CONST_K 0.01
#define CONST_N 4
//delta q == 0.1*part_size
#define DELTA_Q 0.1
#define CONST_C 0.01

class GridNeighbor;
class Box2D;
//TODO: CHANGE IT TO A VIRTUAL CLASS AFTER THE FIRST MILESTONE
#pragma once
struct Particle {
	glm::vec3 pos, vel, new_pos,delta;
	unsigned char r, g, b, a; // Color
	float size;
	float life; // Remaining life of the particle. if < 0 : dead and unused.
	float mass;
	float cameradistance;
	float lambda;
	bool operator<(Particle& that) {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

#pragma once
class Particles {
public:
	Box2D* container;
	GridNeighbor* grid;
	Particle ParticlesContainer[MAX_PARTICLES];
	Shader* shader;
	glm::vec3 translation;
	int ParticlesCount;
	Particles(GLuint particleTexture, Shader* particleShader, glm::vec3 pos);
	~Particles();
	int findUnusedParticle();
	void move_to(glm::vec3 move);
	/// better be called before swapping buffer
	void update();
	void draw();
	void reinitParticle(Particle&);
	GLuint Texture;
	GLuint billboard_vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;
	// controls
};

#endif