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
//void SortParticles() {
//	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
//}

// Finds a Particle in ParticlesContainer which isn't used yet.
// (i.e. life < 0);
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

	return -1; // All particles are taken, override the first one
}

Particles::Particles(GLuint particleTexture, Shader* particleShader, glm::vec3 pos) {
	this->ParticlesCount = 0;
	container = new Box2D({ BOX_SIDE_LENGTH / 2,BOX_SIDE_LENGTH / 2,0 }, { -BOX_SIDE_LENGTH / 2,-BOX_SIDE_LENGTH / 2,0 });

	grid = new GridNeighbor(PARTICLE_SIZE, BOX_SIDE_LENGTH);
	this->shader = particleShader;
	translation = pos;
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

void Particles::move_to(glm::vec3 move) {
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
		grid->remove_part(p->pos.x, p->pos.y, particleIndex);
		reinitParticle(*p);
		grid->add_part(p->pos.x, p->pos.y, particleIndex);
	}
}

void Particles::update() {
	float delta = 0.016f;
	////calculate #of new particles
	//int newparticles = (int)(delta * 10000.0);
	//if (newparticles > (int)(0.016f * 10000.0))
	//	newparticles = (int)(0.016f * 10000.0);
	////reinitialize died/unused particles
	//for (int i = 0; i < newparticles; i++) {
	//	int particleIndex = findUnusedParticle();
	//	if (particleIndex >= 0) {
	//		Particle * p = &ParticlesContainer[particleIndex];
	//		grid->remove_part(p->pos.x, p->pos.y, particleIndex);
	//		reinitParticle(*p);
	//		grid->add_part(p->pos.x, p->pos.y, particleIndex);
	//	}

	//}

	// calculate initial new position
	for (int i = 0; i < MAX_PARTICLES; i++) {

		Particle& p = ParticlesContainer[i]; // shortcut

		if (p.life > delta) {
				p.vel += glm::vec3(0.0f, -GRAVITY, 0.0f) * (float)delta*0.5;
				if (DEBUG)
					cout << "index i : " << i << "vel " << " x " << p.vel.x << p.vel.y << endl;
				p.new_pos = p.pos + p.vel * (float)delta;
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
			if (p.life > delta) {
				//need to check if delta is defined?
				p.new_pos += p.delta;
			}
        }
	}
	grid->update_velocity(ParticlesContainer, delta);
	//update vel and pos
	ParticlesCount = 0;
	for (int i = 0; i < MAX_PARTICLES; i++) {
		Particle& p = ParticlesContainer[i]; // shortcut
		if (p.life > delta) {
			//for bin moving
			float old_x = p.pos.x;
			float old_y = p.pos.y;
			int old_bin_x = floor(p.pos.x + (BOX_SIDE_LENGTH / 2) / grid->bin_size);
			int old_bin_y = floor(p.pos.y + (BOX_SIDE_LENGTH / 2) / grid->bin_size);

			container->in_box(&p);
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
			int new_bin_x = floor(p.pos.x + (BOX_SIDE_LENGTH / 2) / grid->bin_size);
			int new_bin_y = floor(p.pos.y + (BOX_SIDE_LENGTH / 2) / grid->bin_size);

			if (new_bin_x != old_bin_x || new_bin_y != old_bin_y) {
				grid->remove_part(old_x, old_y, i);
				grid->add_part(p.pos.x, p.pos.y, i);
			}
		}
		p.life -= delta;
	}
	//cout << "pc" << ParticlesCount << endl;



	
}

void Particles::reinitParticle(Particle& p) {
	p.life = 30.0f;
	p.pos = translation;
	p.new_pos = translation;
	p.delta = { 0,0,0 };
	p.lambda = 0;
	p.mass = 1;
	float spread = 3.0f;
	glm::vec3 maindir = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 randomdir = glm::vec3(
		(rand() % 2000 - 1000.0f) / 2000.0f,
		(rand() % 2000 - 1000.0f) / 2000.0f,
		0
	);
	if(DEBUG)
		p.vel = { 0,0,0 };
	else
		p.vel = maindir + randomdir*spread;

	// Very bad way to generate a random color
	p.r = rand() % 120 + 134;
	p.g = rand() % 255;
	p.b = rand() % 120+134;
	p.a = 255;

	p.size = PARTICLE_SIZE;
}

void Particles::draw() {

	// fragment shader
	//GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	//printf("%d ",ParticlesCount);


	// Update the buffers that OpenGL uses for rendering.
	// There are much more sophisticated means to stream data from the CPU to the GPU, 
	// but this is outside the scope of this tutorial.
	// http://www.opengl.org/wiki/Buffer_Object_Streaming

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

	// These functions are specific to glDrawArrays*Instanced*.
	// The first parameter is the attribute buffer we're talking about.
	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	// Draw the particules !
	// This draws many times a small triangle_strip (which looks like a quad).
	// This is equivalent to :
	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
	// but faster.
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}