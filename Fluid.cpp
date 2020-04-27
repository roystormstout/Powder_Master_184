#include "Fluid.h"
#include <iostream>
#include <iomanip>
const int damping = 0.99;
using namespace std;

void Fluid::update(float dx) {
	vector<float> buffer(fluid_height * fluid_width, 0.0);
	advect(pressure, buffer, dx);
	pressure = buffer;
	vector<float> xvel_buffer(fluid_height * fluid_width, 0.0);
	vector<float> yvel_buffer(fluid_height * fluid_width, 0.0);
	advect(xvel, xvel_buffer, dx);
	advect(yvel, yvel_buffer, dx);
	print_buffer(xvel_buffer);
	xvel = xvel_buffer;
	print_buffer(xvel_buffer);
	yvel = yvel_buffer;
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
			int x0 = floor(x);
			int x1 = ceil(x);
			int y0 = floor(y);
			int y1 = ceil(y);
			if (in_bounds(x0, y0)) {
				cell = get_cell(x0, y0);
				float weight = abs((x0 - x) * (y0 - y));
				buffer[cell] += weight * field[cell];
			}
			if (in_bounds(x0, y1)) {
				cell = get_cell(x0, y1);
				float weight = abs((x0 - x) * (y1 - y));
				buffer[cell] += weight * field[cell];
			}
			if (in_bounds(x1, y0)) {
				cell = get_cell(x1, y0);
				float weight = abs((x1 - x) * (y0 - y));
				buffer[cell] += weight * field[cell];
			}
			if (in_bounds(x1, y1)) {
				cell = get_cell(x1, y1);
				float weight = abs((x1 - x) * (y1 - y));
				buffer[cell] += weight * field[cell];
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
	y += 1.0;
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
		float weight = abs((x0 - x) * (y0 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	if (in_bounds(x0, y1)) {
		cell = get_cell(x0, y1);
		float weight = abs((x0 - x) * (y1 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	if (in_bounds(x1, y0)) {
		cell = get_cell(x1, y0);
		float weight = abs((x1 - x) * (y0 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
	if (in_bounds(x1, y1)) {
		cell = get_cell(x1, y1);
		float weight = abs((x1 - x) * (y1 - y));
		vel[0] += xvel[cell] * weight;
		vel[1] += yvel[cell] * weight;
	}
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
			cout << xvel[cell] << " ";
		}
		cout << "\n";
	}
}
