//
// Created by daria on 27/11/2023.
//

#include "PerlinNoise.h"
#include <cmath>

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

double PerlinNoise::noise2d(const double& x, const double& y){
    const int X = static_cast<int>(std::floor(x)) & 255;
    const int Y = static_cast<int>(std::floor(y)) & 255;
    const double xf = x-std::floor(x);
    const double yf = y-std::floor(y);

    auto topRight = std::make_pair(xf-1.0, yf-1.0);
    auto topLeft = std::make_pair(xf, yf-1.0);
    auto bottomRight = std::make_pair(xf-1.0, yf);
    auto bottomLeft = std::make_pair(xf, yf);

    auto valueTopRight = P[P[X+1]+Y+1];
    auto valueTopLeft = P[P[X]+Y+1];
    auto valueBottomRight = P[P[X+1]+Y];
    auto valueBottomLeft = P[P[X]+Y];
}