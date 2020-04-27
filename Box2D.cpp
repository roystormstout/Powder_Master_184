#include "Box2D.h"


Box2D::Box2D(glm::vec3 max_point, glm::vec3 min_point) {
	this->max = max_point;
	this -> min = min_point;
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