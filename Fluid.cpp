#include "Fluid.h"
#include <iostream>
#include <iomanip>
const float damping = 0.999;
using namespace std;

void Fluid::update(float dt) {
	//first, perform advection on all fields
	vector<float> buffer(fluid_height * fluid_width, 0.0);
	advect(pressure, buffer, dt);
	pressure = buffer;
	vector<float> xvel_buffer(fluid_height * fluid_width, 0.0);
	vector<float> yvel_buffer(fluid_height * fluid_width, 0.0);
	advect(xvel, xvel_buffer, dt);
	advect(yvel, yvel_buffer, dt);
	xvel = xvel_buffer;
	yvel = yvel_buffer;

	//next, apply force due to pressure
	apply_pressure(dt);

	//finally, applying damping constant
	for (int i = 0; i < xvel.size(); i++) {
		xvel[i] *= damping;
		yvel[i] *= damping;
	}
}

void Fluid::apply_pressure(float dt) {
	for (int x = 0;  x < fluid_width - 1; x++) {
		for (int y = 0; y < fluid_height - 1; y++) {
			int cell = get_cell(x, y);
			float fx = pressure[cell] - pressure[cell + 1];
			float fy = pressure[cell] - pressure[cell + fluid_width];
			fx *= dt * 1;
			fy *= dt * 1;
			xvel[cell] += fx;
			xvel[cell + 1] += fx;
			yvel[cell] += fy;
			yvel[cell + fluid_width] += fy;
		}
	}
}

int Fluid::get_cell(int x, int y) {
	return x + y * fluid_width;
}

//advect the field using xvel and yvel, storing the result in the buffer
void Fluid::advect(vector<float> &field, vector<float> &buffer, float dx) {
	for (int i = 0; i < fluid_width; i++) {
		for (int j = 0; j < fluid_height; j++) {
			int cell = get_cell(i, j);
			float x = i + xvel[cell] * dx;
			float y = j + yvel[cell] * dx;
			float field_val = field[cell];
			//do bilinear interpolation to split value into neighboring cells
			float x0 = floor(x);
			float x1 = x0 + 1;
			float y0 = floor(y);
			float y1 = y0 + 1;
			float w0 = 0.0, w1 = 0.0, w2 = 0.0, w3 = 0.0;
			if (in_bounds(x0, y0)) {
				w0 = abs((x1 - x) * (y1 - y));
			}
			if (in_bounds(x0, y1)) {
				w1 = abs((x1 - x) * (y0 - y));
			}
			if (in_bounds(x1, y0)) {
				w2 = abs((x0 - x) * (y1 - y));
			}
			if (in_bounds(x1, y1)) {
				w3 = abs((x0 - x) * (y0 - y));
			}
			float total_weight = w0 + w1 + w2 + w3;
			if (in_bounds(x0, y0)) {
				buffer[get_cell(x0, y0)] += w0 / total_weight * field[cell];
			}
			if (in_bounds(x0, y1)) {
				buffer[get_cell(x0, y1)] += w1 / total_weight * field[cell];
			}
			if (in_bounds(x1, y0)) {
				buffer[get_cell(x1, y0)] += w2 / total_weight * field[cell];
			}
			if (in_bounds(x1, y1)) {
				buffer[get_cell(x1, y1)] += w3 / total_weight * field[cell];
			}
		}
	}
}

bool Fluid::in_bounds(int x, int y) {
	return x >= 0 && x < fluid_width && y >= 0 && y < fluid_height;
}

void Fluid::draw() {
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	for (int i = 0; i < fluid_width; i++) {
		for (int j = 0; j < fluid_height; j++) {
			int cell = get_cell(i, j);
		}
	}
}

glm::vec3 Fluid::get_vel(float x, float y) {
	x += 1.0;
	y = 1 - y;
	x /= cell_size;
	y /= cell_size;
	int cell = get_cell(x, y);
	glm::vec3 vel = glm::vec3(0.0, 0.0, 0.0);;
	//do bilinear interpolation to sample vel
	int x0 = floor(x);
	int x1 = ceil(x);
	int y0 = floor(y);
	int y1 = ceil(y);

	if (in_bounds(x0, y0)) {
		cell = get_cell(x0, y0);
		float weight = abs((x1 - x) * (y1 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	if (in_bounds(x0, y1)) {
		cell = get_cell(x0, y1);
		float weight = abs((x1 - x) * (y0 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	if (in_bounds(x1, y0)) {
		cell = get_cell(x1, y0);
		float weight = abs((x0 - x) * (y1 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	if (in_bounds(x1, y1)) {
		cell = get_cell(x1, y1);
		float weight = abs((x0 - x) * (y0 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	vel *= -1;
	return vel;
}


void Fluid::debug() {
	print_buffer(xvel);
}

void Fluid::print_buffer(vector<float> &buf) {
	cout << setprecision(3);
	for (int i = 0; i < fluid_width; i++) {
		for (int j = 0; j < fluid_height; j++) {
			int cell = get_cell(i, j);
			cout << buf[cell] << " ";
		}
		cout << "\n";
	}
}
