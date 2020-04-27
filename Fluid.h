#pragma once
#include <vector>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include "glm/ext.hpp"

using namespace std;
const int cell_size = 64;
class Fluid
{
private:
	vector<float> pressure;
	vector<float> xvel;
	vector<float> yvel;
	int fluid_width;
	int fluid_height;
	int screen_width;
	int screen_height;
	void advect(vector<float> &field, vector<float> &buffer, float dx);
	int get_cell(int x, int y);
	bool in_bounds(int x, int y);
	void print_buffer(vector<float>& buf);
public:
	Fluid(int width, int height) {
		fluid_width = width / cell_size + 1;
		fluid_height = height / cell_size + 1;
		screen_width = width;
		screen_height = height;
		pressure.assign(fluid_width * fluid_height, 1.0);
		xvel.assign(fluid_width * fluid_height, 0.1);
		yvel.assign(fluid_width * fluid_height, 0.0);
	}
	void update(float dx);
	void debug();
	void draw();
	glm::vec3 get_vel(float x, float y);
};

