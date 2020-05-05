#ifndef _GRID_NEIGHBOR_H
#define _GRID_NEIGHBOR_H


#include <cmath>
#include <vector>
#include <list>
#include <unordered_map>
#include <stdio.h>
#include <iostream>
#include "Particle.h"
using namespace std;

typedef list<int> bin_t;

#pragma once
class GridNeighbor {
	//assume rectangular and axis aligned
public:
	int num_bins;
	double bin_size;
	vector<bin_t> grid;

	GridNeighbor(double part_size, double size);
	~GridNeighbor(void) {};
	void add_part( float x,float y, int part_index);
	void assign_parts(Particle* parts, int num_parts);
	void remove_part(float x, float y, int part_index);
	void calculate_lambda(Particle* parts);
	void calculate_delta(Particle* parts);
	void update_velocity(Particle* parts, float delta);
};
#endif