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
#include "fluid.h"

enum PART_TYPE { water, rock };

#define DEBUG false
#define FLUID_ENABLED true
#define BINNING_OPTIMIZED true
#define TENSILE_INSTABILITY true
#define VORTICITY_EFFECT false
#define VISCOSITY_EFFECT true

#define FLUID_COEFF 0.5f
#define DELTA_TIME 0.016f
#define FLUID_ITER 5
//number of fluid cells
#define FLUID_HEIGHT 8
#define FLUID_WIDTH 8
//particle config
#define MAX_PARTICLES 300
#define PARTICLE_SIZE 0.2
#define PARTICLE_LIFE 500
#define REST_DENSITY 800
#define PI 3.14159265
#define GRAVITY 1
#define SOLVER_ITER 3
#define BOX_SIDE_LENGTH 6.0f
#define EPSILON 100.0f
//for tensile instability
#define CONST_K 0.01
#define CONST_N 4
//delta q == 0.1*part_size
#define DELTA_Q 0.01
#define CONST_C 0.01

#define DRAW_RADIUS 0.01
class GridNeighbor;
class Box2D;
//TODO: CHANGE IT TO A VIRTUAL CLASS AFTER THE FIRST MILESTONE
#pragma once
struct Particle {
	glm::vec3 pos, vel, new_pos, delta, force;
	unsigned char r, g, b, a; // Color
	float size;
	float life; // Remaining life of the particle. if < 0 : dead and unused.
	float mass;
	float cameradistance;
	float lambda;
	PART_TYPE type;
	bool operator<(Particle& that) {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

#pragma once
class Particles {
public:
	GridNeighbor* grid;
	Particle ParticlesContainer[MAX_PARTICLES];
	Shader* shader;
	glm::vec3 translation;
	int ParticlesCount;
	Fluid* fluid;
	Particles(GLuint particleTexture, Shader* particleShader, Fluid* f);
	~Particles();
	int findUnusedParticle();
	void spawn_at(glm::vec3 move, glm::vec3 color, glm::vec3 force, PART_TYPE type);
	/// better be called before swapping buffer
	void update();
	void draw();
	void initParticle(Particle& p, glm::vec3 color, glm::vec3 force, PART_TYPE type);
	void bind_fluid(Fluid* fluid);
	GLuint Texture;
	GLuint billboard_vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;
	// controls
};

#endif