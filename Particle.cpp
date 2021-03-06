#pragma warning(disable : 4996)
#include "Particle.h"
#include "Window.h"
#include "Box2D.h"
#include "GridNeighbor.h"

static GLfloat g_vertex_buffer_data[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f,
	0.5f,  0.5f, 0.0f,
};
static GLfloat* g_particule_position_size_data = new GLfloat[MAX_PARTICLES * 4];
static GLubyte* g_particule_color_data = new GLubyte[MAX_PARTICLES * 4];

int LastUsedParticle = 0;


int  Particles::findUnusedParticle() {

	for (int i = LastUsedParticle; i< MAX_PARTICLES; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return -1; // All particles are taken, failed to create new particles
}

Particles::Particles(GLuint particleTexture, Shader* particleShader, Fluid* f) {
	this->ParticlesCount = 0;
	grid = new GridNeighbor(PARTICLE_SIZE, BOX_SIDE_LENGTH);
	this->shader = particleShader;
	translation = { 0,0,0 };
	bind_fluid(f);
	// Create and compile our GLSL program from the shaders
	auto programID = shader->ID;
	// Vertex shader
	GLuint CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	// fragment shader
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");


	static GLfloat* g_particule_position_size_data = new GLfloat[MAX_PARTICLES * 4];
	static GLubyte* g_particule_color_data = new GLubyte[MAX_PARTICLES * 4];

	for (int i = 0; i< MAX_PARTICLES; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	Texture = particleTexture;

	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

}

Particles::~Particles() {}

void Particles::spawn_at(glm::vec3 move, glm::vec3 color, glm::vec3 force, PART_TYPE type) {
	if (move.x > BOX_SIDE_LENGTH/2.0f)
		move.x = BOX_SIDE_LENGTH / 2.0001f;
	else if(move.x < -BOX_SIDE_LENGTH / 2.0f)
		move.x = -BOX_SIDE_LENGTH / 2.0f;
	if (move.y > BOX_SIDE_LENGTH / 2.0f)
		move.y = BOX_SIDE_LENGTH / 2.0001f;
	else if (move.y < -BOX_SIDE_LENGTH / 2.0f)
		move.y = -BOX_SIDE_LENGTH / 2.0f;

	translation = move;
	int particleIndex = findUnusedParticle();
	if (particleIndex >= 0) {
		Particle* p = &ParticlesContainer[particleIndex];
		//grid->remove_part(p->pos.x, p->pos.y, particleIndex);
		initParticle(*p, color, force, type);
		grid->add_part(move.x, move.y, particleIndex);
	}
}

void Particles::update() {

	// calculate initial new position
	for (int i = 0; i < MAX_PARTICLES; i++) {

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > DELTA_TIME) {
			if (p.type == water) {
				//glm::vec3 new_force = fluid->get_vel(p.pos.x, p.pos.y) + p.force;
				p.vel += p.force * (float)DELTA_TIME;
				if(FLUID_ENABLED)
					p.vel += fluid->get_vel(p.pos.x, p.pos.y)*FLUID_COEFF;
			}
				//p.vel += fluid->get_vel(p.pos[0], p.pos[1]);
				if (DEBUG)
					cout << "index i : " << i << "vel " << " x " << p.vel.x << p.vel.y << endl;
				p.new_pos = p.pos + p.vel * (float)DELTA_TIME;
		}
		else {
			grid->remove_part(p.pos.x, p.pos.y, i);
			p.cameradistance = -1.0f;
			//reinitParticle(p);
		}
	}

	for (int j = 0; j < SOLVER_ITER; j++) {
		grid->calculate_lambda(ParticlesContainer);
		grid->calculate_delta(ParticlesContainer);
		for (int i = 0; i < MAX_PARTICLES; i++) {
			Particle& p = ParticlesContainer[i]; // shortcut
			if (p.life > DELTA_TIME && p.type == water) {
				p.new_pos += p.delta;
			}
        }
	}
	grid->update_velocity(ParticlesContainer, DELTA_TIME);
	//update vel and pos
	ParticlesCount = 0;
	for (int i = 0; i < MAX_PARTICLES; i++) {
		Particle& p = ParticlesContainer[i]; // shortcut
		if (p.life > DELTA_TIME) {
			//for bin moving
			float old_x = p.pos.x;
			float old_y = p.pos.y;
			int old_bin_x = floor((p.pos.x + (BOX_SIDE_LENGTH / 2)) / grid->bin_size);
			int old_bin_y = floor((p.pos.y + (BOX_SIDE_LENGTH / 2)) / grid->bin_size);

			Window::scene->container->in_box(&p);
			p.pos = p.new_pos;
			p.cameradistance = glm::length(p.pos - Window::scene->camera->cam_pos);
			//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

			// Fill the GPU buffer
			g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
			g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
			g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

			g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

			g_particule_color_data[4 * ParticlesCount + 0] = p.r;
			g_particule_color_data[4 * ParticlesCount + 1] = p.g;
			g_particule_color_data[4 * ParticlesCount + 2] = p.b;
			g_particule_color_data[4 * ParticlesCount + 3] = p.a;

			ParticlesCount++;
			// update bin assignment if necessary
			int new_bin_x = floor((p.pos.x + (BOX_SIDE_LENGTH / 2)) / grid->bin_size);
			int new_bin_y = floor((p.pos.y + (BOX_SIDE_LENGTH / 2)) / grid->bin_size);

			if (new_bin_x != old_bin_x || new_bin_y != old_bin_y) {
				grid->remove_part(old_x, old_y, i);
				grid->add_part(p.pos.x, p.pos.y, i);
			}
		}
		p.life -= DELTA_TIME;
	}
	//cout << "pc" << ParticlesCount << endl;



	
}

void Particles::initParticle(Particle& p, glm::vec3 color, glm::vec3 force, PART_TYPE type) {
	p.life = PARTICLE_LIFE;
	p.pos = translation;
	p.new_pos = translation;
	p.delta = { 0,0,0 };
	p.force = force;
	p.lambda = 0;
	p.mass = 1;
	p.vel = { 0,0,0 };
	p.type = type;
	p.r = color.r;
	p.g = color.g;
	p.b = color.b;
	p.a = 255;

	p.size = PARTICLE_SIZE;
}

void Particles::draw() {

	// fragment shader
	//GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	auto programID = shader->ID;

	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Use our shader
	shader->use();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	shader->setInt("myTextureSampler", 0);
	//glUniform1i(TextureID, 0);

	// Same as the billboards tutorial
	glm::mat4 viewMtx = Window::scene->camera->GetViewMtx();
	shader->setVec3("CameraRight_worldspace", viewMtx[0][0], viewMtx[1][0], viewMtx[2][0]);
	shader->setVec3("CameraUp_worldspace", viewMtx[0][1], viewMtx[1][1], viewMtx[2][1]);
	shader->setMat4("VP", Window::scene->camera->GetViewProjectMtx());

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// 2nd attribute buffer : positions of particles' centers
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : x + y + z + size => 4
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);

	// 3rd attribute buffer : particles' colors
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		4,                                // size : r + g + b + a => 4
		GL_UNSIGNED_BYTE,                 // type
		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
		0,                                // stride
		(void*)0                          // array buffer offset
	);


	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

//TODO: this is kind of hacky, find a better way to expose fluid object
void Particles::bind_fluid(Fluid* f) {
	fluid = f;
}