//
// Created by daria on 27/11/2023.
//

#include "PerlinNoise.h"

double PerlinNoise::noise3d(const double& x, const double& y, const double& z){
    double AB = noise2d(x,y);
    double BC = noise2d(y,z);
    double AC = noise2d(x,z);

    double BA = noise2d(y,x);
    double CB = noise2d(z,y);
    double CA = noise2d(z,x);

    double ABC = AB + BC + AC + BA + CB + CA;
    return ABC/6.0;
}