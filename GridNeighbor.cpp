#include "GridNeighbor.h"
#include "Window.h"

float W(glm::vec3 r) {
    // based on poly6
    float multiplier = 0.0f;
    if (0.0f <= glm::length(r) && glm::length(r) <= PARTICLE_SIZE) {
        multiplier = pow(pow(PARTICLE_SIZE, 2.0f) - pow(glm::length(r), 2.0f), 3.0f);
    }
    float res = (315.0f / (64.0f * PI * pow(PARTICLE_SIZE, 9.0f))) * multiplier;
    return res;
}


glm::vec3 deltaW(glm::vec3 r) {
    glm::vec3 multiplier{ 0,0,0 };
    if (0.0f < glm::length(r) && glm::length(r) <= PARTICLE_SIZE) {
        multiplier = (r / glm::length(r)) * pow(PARTICLE_SIZE - glm::length(r), 2.0f);
    }
    return (multiplier * (45.0f / (PI * pow(PARTICLE_SIZE, 6.0f))));
}


GridNeighbor::GridNeighbor(double part_size, double size) {
    if (BINNING_OPTIMIZED) {
        bin_size = part_size;
        num_bins = ceil(size / bin_size);
    }
    else {
        bin_size = size;
        num_bins = 1;
    }
	grid.resize(num_bins * num_bins);
}


void GridNeighbor::add_part( float x, float y, int part_index) {
    // BUG: x and y may get closer to the wall than they should 
    int bin_x = floor((x + (BOX_SIDE_LENGTH / 2.0f)) / bin_size);
    int bin_y = floor((y + (BOX_SIDE_LENGTH / 2.0f)) / bin_size);
    if (bin_x >= num_bins)
        bin_x = num_bins - 1;
    else if (bin_x < 0)
        bin_x = 0;
    if (bin_y >= num_bins)
        bin_y = num_bins - 1;
    else if (bin_y < 0)
        bin_y = 0;
    grid[bin_x * num_bins + bin_y].push_back(part_index);
}

// called once to assign particles to bins
void GridNeighbor::assign_parts(Particle* parts, int num_parts) {
    for (int i = 0; i < num_parts; ++i) {
        int bin_x = (int)((parts[i].pos.x+ (BOX_SIDE_LENGTH / 2.0f)) / bin_size);
        int bin_y = (int)((parts[i].pos.y + (BOX_SIDE_LENGTH / 2.0f)) / bin_size);
        grid[bin_x * num_bins + bin_y].push_back(i);
    }
}

// remove particle from the bin at grid[x][y]
void GridNeighbor::remove_part(float x, float y, int part_index) {
    int bin_x = floor((x + (BOX_SIDE_LENGTH / 2.0f)) / bin_size);
    int bin_y = floor((y + (BOX_SIDE_LENGTH / 2.0f)) / bin_size);
    if (bin_x >= num_bins)
        bin_x = num_bins - 1;
    else if (bin_x < 0)
        bin_x = 0;
    if (bin_y >= num_bins)
        bin_y = num_bins - 1;
    else if (bin_y < 0)
        bin_y = 0;
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
                if (DEBUG)
                   cout << " i " << i << " in bin " << bin_x * num_bins + bin_y << endl;
                float density_i = 0.0f;
                float density_0 = REST_DENSITY; 
                for (int bi = top; bi <= bottom; ++bi) {
                    for (int bj = left; bj <= right; ++bj) {
                        bin_t& curr_bin = grid[bi * num_bins + bj];
                        for (auto cit = curr_bin.begin(); cit != curr_bin.end(); ++cit) {
                            if (i != *cit) {
                                if (DEBUG)
                                    cout << "neighbor for i " << i << " are " << *cit << endl;
                                glm::vec3 diff_ij = parts[i].new_pos - parts[*cit].new_pos;
                                float y = W(diff_ij);
                               /* glm::vec3 delta_y = -deltaW(diff_ij);
                                multiplier += delta_y;*/
                                density_i += parts[*cit].mass * y;
                                //if (DEBUG)
                                //    cout << "delta y for i " << i << " are " << delta_y.y << endl;
                                //denom_sum += pow(glm::length((-delta_y * (1 / density_0))), 2);
                             
                            }
                        }
                    }
                }
                float C_i = (density_i / density_0) - 1.0f;
                float denom_sum = 0.0f;
                glm::vec3 multiplier_self({ 0,0,0 });
                for (int bi = top; bi <= bottom; ++bi) {
                    for (int bj = left; bj <= right; ++bj) {
                        bin_t& curr_bin = grid[bi * num_bins + bj];
                        for (auto cit = curr_bin.begin(); cit != curr_bin.end(); ++cit) {
                            glm::vec3 multiplier({ 0,0,0 });
                            if (i != *cit) {
                                glm::vec3 diff_ij = parts[i].new_pos - parts[*cit].new_pos;
                                glm::vec3 x = deltaW(diff_ij);
                                multiplier = x;
                                multiplier_self += x;
                                auto deltaC_i = multiplier * (1.0f / density_0);
                                denom_sum += pow(glm::length(deltaC_i), 2);
                            }
                        }
                    }
                }
                auto deltaC_i = multiplier_self * (1.0f / density_0);
                denom_sum += pow(glm::length(deltaC_i), 2.0);
                if (DEBUG) {
                    cout << "part i and denom_sum " << i << "  " << denom_sum << " and Ci is " << C_i << endl;
                    
                }
                parts[i].lambda = -(C_i / (denom_sum+EPSILON));
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
                                if (DEBUG)
                                    cout << "part i and neighbor" << i << "  " << *cit << " diff=  " << diff_ij.x << diff_ij.y << endl;
                                float s_corr = 0.0f;
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


void GridNeighbor::update_velocity(Particle* parts, float delta) {
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
                parts[i].vel = (parts[i].new_pos - (parts[i].pos)) / delta;
                if (VORTICITY_EFFECT) {
                    glm::vec3 omega({ 0,0,0 });
                    for (int bi = top; bi <= bottom; ++bi) {
                        for (int bj = left; bj <= right; ++bj) {
                            bin_t& curr_bin = grid[bi * num_bins + bj];
                            for (auto cit = curr_bin.begin(); cit != curr_bin.end(); ++cit) {
                                if (i != *cit) {
                                    glm::vec3 diff_ij = parts[i].new_pos - parts[*cit].new_pos;
                                    glm::vec3 vel_diff = parts[*cit].vel - parts[i].vel;
                                    omega += (vel_diff * deltaW(diff_ij));
                                }
                            }
                        }
                    }
                    glm::vec3 eta({ glm::length(omega),glm::length(omega), glm::length(omega) });
                    if (glm::length(eta) > 0) {
                        glm::vec3 N = eta / glm::length(eta);
                        parts[i].force += glm::cross(N, omega)/ EPSILON;
                        cout << "force x " << parts[i].force.x << "  y  " << parts[i].force.y << endl;
                    }
                }
                if (VISCOSITY_EFFECT) {
                    glm::vec3 new_vel_sum({ 0,0,0 });
                    for (int bi = top; bi <= bottom; ++bi) {
                        for (int bj = left; bj <= right; ++bj) {
                            bin_t& curr_bin = grid[bi * num_bins + bj];
                            for (auto cit = curr_bin.begin(); cit != curr_bin.end(); ++cit) {
                                if (i != *cit) {
                                    glm::vec3 vel_diff = parts[*cit].vel - parts[i].vel;
                                    new_vel_sum += vel_diff * W(parts[*cit].pos - parts[i].pos);
                                }
                            }
                        }
                    }
                    parts[i].vel += new_vel_sum *CONST_C;
                }
            }
        }
    }
}