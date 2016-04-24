//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
//

#include "arealight.h"

std::vector<Point> AreaLight::sample(int sampling_num) {
    std::vector<Point> samples;

    if (sampling_num < 1) {
        std::cerr << "Sampling Number in Area Light is less than 1! Automatically make it 1 then" << std::endl;
        sampling_num = 1;
    }

    float half_len = _len * 0.5f;
    float step_size = _len / sampling_num;
    for (int p = 0; p < sampling_num; p++) {
        for (int q = 0; q < sampling_num; q++) {
            // stratified
            Point sample = _orig + _u * (q * step_size + rand_float() * step_size - half_len) +
                           _v * (p * step_size + rand_float() * step_size - half_len);
            samples.push_back(std::move(sample));
        }
    }
    return std::move(samples);
}