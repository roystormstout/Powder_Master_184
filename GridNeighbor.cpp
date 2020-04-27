#include "GridNeighbor.h"
#include "Window.h"

float W(glm::vec3 r) {
    // based on poly6
    float multiplier = 0.0f;
    if (0.0f <= r.length() && r.length() <= PARTICLE_SIZE) {
        multiplier = pow(pow(PARTICLE_SIZE, 2.0f) - pow(r.length(), 2.0f), 3.0f);
    }
    float res = (315.0f / (64.0f * PI * pow(PARTICLE_SIZE, 9.0f))) * multiplier;
    return res;
}


glm::vec3 deltaW(glm::vec3 r) {
    glm::vec3 multiplier{ 0,0,0 };
    if (0.0f <= r.length() && r.length() <= PARTICLE_SIZE) {
        multiplier = (r / abs(r.length())) * pow(PARTICLE_SIZE - r.length(), 2.0f);
    }
    return multiplier * (45.0f / (PI * pow(PARTICLE_SIZE, 6.0f)));
}


GridNeighbor::GridNeighbor(double part_size, double size) {
	bin_size = part_size;
	num_bins = ceil(size / bin_size);
	grid.resize(num_bins * num_bins);
}


void GridNeighbor::add_part( float x, float y, int part_index) {
    int bin_x = (x + (BOX_SIDE_LENGTH / 2)) / bin_size;
    int bin_y = (y + (BOX_SIDE_LENGTH / 2)) / bin_size;
    grid[bin_x * num_bins + bin_y].push_back(part_index);
}

// called once to assign particles to bins
void GridNeighbor::assign_parts(Particle* parts, int num_parts, vector<bin_t>& grid, double bin_size) {
    for (int i = 0; i < num_parts; ++i) {
        int bin_x = (int)((parts[i].pos.x+ (BOX_SIDE_LENGTH / 2)) / bin_size);
        int bin_y = (int)((parts[i].pos.y + (BOX_SIDE_LENGTH / 2)) / bin_size);
        grid[bin_x * num_bins + bin_y].push_back(i);
    }
}

// remove particle from the bin at grid[x][y]
void GridNeighbor::remove_part(float x, float y, int part_index) {
    int bin_x = (x+ (BOX_SIDE_LENGTH / 2)) / bin_size;
    int bin_y = (y + (BOX_SIDE_LENGTH / 2)) / bin_size;
    bin_t& vect = grid[bin_x * num_bins + bin_y];
    //for(int i=0; i<vect.size(); ++i){
    int i = 0;
    for (auto it = vect.begin(); it != vect.end() && i < vect.size(); ++it) {
        //if(vect[i] == part_index){
        if (*it == part_index) {
            //vect.erase(vect.begin()+i);
            vect.erase(it);
            break;
        }
        i++;
    }
}

void GridNeighbor::calculate_lambda(Particle* parts) {
    // Compute Forces
    for (int bin_x = 0; bin_x < num_bins; ++bin_x) {
        int top = bin_x - 1;
        if (top < 0) top = 0;
        int bottom = bin_x + 1;
        if (bottom > num_bins - 1) bottom = num_bins - 1;
        for (int bin_y = 0; bin_y < num_bins; ++bin_y) {
            int left = bin_y - 1;
            if (left < 0)  left = 0;
            int right = bin_y + 1;
            if (right > num_bins - 1) right = num_bins - 1;

            bin_t& bin = grid[bin_x * num_bins + bin_y];
            for (auto pi = bin.begin(); pi != bin.end(); ++pi) {
                int i = *pi;
                float density_i = 0;
                float density_0 = REST_DENSITY; 
                glm::vec3 multiplier({ 0,0,0 });
                float denom_sum = 0.0f;
                for (int bi = top; bi <= bottom; ++bi) {
                    for (int bj = left; bj <= right; ++bj) {
                        bin_t& curr_bin = grid[bi * num_bins + bj];
                        for (auto cit = curr_bin.begin(); cit != curr_bin.end(); ++cit) {
                            if (i != *cit) {
                                glm::vec3 diff_ij = parts[i].new_pos - parts[*cit].new_pos;
                                float y = W(diff_ij);
                                glm::vec3 delta_y = deltaW(diff_ij);
                                multiplier += delta_y;
                                density_i += parts[*cit].mass * y;

                                denom_sum += pow((-delta_y * (1 / density_0)).length(), 2);
                            }
                        }
                    }
                }
                float C_i = (density_i / density_0) - 1.0f;
                auto deltaC_i = multiplier * (1.0f / density_0);
                denom_sum += pow(deltaC_i.length(), 2);
                parts[i].lambda = -1.0f * (C_i / (denom_sum));
                if (DEBUG)
                    cout << "part i and lambda" << i << "  " << parts[i].lambda << endl;
            }
        }
    }
   
}

void GridNeighbor::calculate_delta(Particle* parts) {
    // Compute Forces
    for (int bin_x = 0; bin_x < num_bins; ++bin_x) {
        int top = bin_x - 1;
        if (top < 0) top = 0;
        int bottom = bin_x + 1;
        if (bottom > num_bins - 1) bottom = num_bins - 1;
        for (int bin_y = 0; bin_y < num_bins; ++bin_y) {
            int left = bin_y - 1;
            if (left < 0)  left = 0;
            int right = bin_y + 1;
            if (right > num_bins - 1) right = num_bins - 1;

            bin_t& bin = grid[bin_x * num_bins + bin_y];
            for (auto pi = bin.begin(); pi != bin.end(); ++pi) {
                int i = *pi;
                glm::vec3 delta_p_i({ 0,0,0 });
                for (int bi = top; bi <= bottom; ++bi) {
                    for (int bj = left; bj <= right; ++bj) {
                        bin_t& curr_bin = grid[bi * num_bins + bj];
                        for (auto cit = curr_bin.begin(); cit != curr_bin.end(); ++cit) {
                            if (i != *cit) {
                                glm::vec3 diff_ij = parts[i].new_pos - parts[*cit].new_pos;
                            
                                float s_corr = 0;
                                if (TENSILE_INSTABILITY) {
                                    float s_corr = pow(-CONST_K * (W(diff_ij) / W({ 0,DELTA_Q,0 } )), CONST_N);
                                }
                                delta_p_i+= deltaW(diff_ij)* (parts[i].lambda + parts[*cit].lambda + s_corr);
                            }
                        }
                    }
                }
                parts[i].delta = delta_p_i*(1.0f / (float)REST_DENSITY);
                if (DEBUG)
                    cout << "part i and delta" << i << "  " << delta_p_i.x << delta_p_i.y << endl;
            }
        }
    }

}