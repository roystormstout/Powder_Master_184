#pragma once
#include <vector>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include "glm/ext.hpp"

using namespace std;
class Fluid
{
private:
	vector<float> pressure;
	vector<float> xvel;
	vector<float> yvel;
	int fluid_width;
	int fluid_height;
	void advect(vector<float> &field, vector<float> &buffer, float dx);
	int get_cell(int x, int y);
	bool in_bounds(int x, int y);
	void print_buffer(vector<float>& buf);
	void apply_pressure(float dt);
public:
	Fluid(int width, int height) {
		fluid_width = width;
		fluid_height = height;
		pressure.assign(fluid_width * fluid_height, 1.0);
		xvel.assign(fluid_width * fluid_height, 0);
		yvel.assign(fluid_width * fluid_height, 0);

		//TEMP DEBUG
		for (int x = 0; x < fluid_width; x++) {
			for (int y = 0; y < fluid_height; y++) {
				int cell = get_cell(x, y);
				float tx = x - (fluid_height - 1) / 2.0;
				float ty = y - (fluid_width - 1) / 2.0;
				xvel[cell] += -ty * 4;
				yvel[cell] += tx * 4;
			}
		}
	}
	void update(float dt);
	void debug();
	void draw();
	glm::vec3 get_vel(float x, float y);
};

