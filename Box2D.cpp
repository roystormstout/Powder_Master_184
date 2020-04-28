#include "Box2D.h"
#include "Window.h"

Box2D::Box2D(glm::vec3 max_point, glm::vec3 min_point, Shader * shader) {
	this->max = max_point;
	this -> min = min_point;
	this->shader = shader;
	glm::vec2 lineVertices[] =
	{
		{max.x+0.25f, max.y + 0.25f},
		{max.x + 0.25f, min.y-0.25f},
		{min.x - 0.25f, min.y - 0.25f},
		{min.x - 0.25f, max.y + 0.25f}
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

bool Box2D::in_box(Particle* part) {
	bool in_cont = true;
	float friction = .5f;

	glm::vec3 pos = part->new_pos;
	glm::vec3 vel = part->vel;

	//reflect x position
	if (pos.x > max.x) {
		pos.x = max.x - (pos.x - max.x);
		vel.x = -vel.x * friction;
		in_cont = false;
	}
	else if (pos.x < min.x) {
		pos.x = min.x + (min.x - pos.x);
		vel.x = -vel.x * friction;
		in_cont = false;
	}

	//reflect y position
	if (pos.y > max.y) {
		pos.y = max.y - (pos.y - max.y);
		vel.y = -vel.y * friction;
		in_cont = false;
	}
	else if (pos.y < min.y) {
		pos.y = min.y + (min.y - pos.y);
		vel.y = -vel.y * friction;
		in_cont = false;
	}

	part->new_pos = pos;
	part->vel = vel;
	return in_cont;
}

void Box2D::draw() {
	shader->use();

	shader->setMat4("VP", Window::scene->camera->GetViewProjectMtx());
	// Now draw the cube. We simply need to bind the VAO associated with it.
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glBindVertexArray(0);
}