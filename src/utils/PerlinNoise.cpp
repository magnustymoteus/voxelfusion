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
std::pair<double,double> PerlinNoise::GetConstantVector(int v) {
    // v is the value from the permutation table
    auto h = v & 3;
    if(h == 0)
        return std::pair(1.0, 1.0);
    else if(h == 1)
        return std::pair(-1.0, 1.0);
    else if(h == 2)
        return  std::pair(-1.0, -1.0);
    else
        return std::pair(1.0, -1.0);
}
double PerlinNoise::lerp(double t, double a, double b) {
    return a + t * (b - a);
}
double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
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

    auto dotTopRight = topRight.first*GetConstantVector(valueTopRight).first + topRight.second*GetConstantVector(valueTopRight).second;
    auto dotTopLeft = topLeft.first*(GetConstantVector(valueTopLeft)).first + topLeft.second*(GetConstantVector(valueTopLeft)).second;
    auto dotBottomRight = bottomRight.first*(GetConstantVector(valueBottomRight)).first + bottomRight.second*(GetConstantVector(valueBottomRight)).second;
    auto dotBottomLeft = bottomLeft.first*(GetConstantVector(valueBottomLeft)).first * bottomLeft.second*(GetConstantVector(valueBottomLeft)).second;

    auto u = fade(xf);
    auto v = fade(yf);
    auto result = Lerp(u,
                        Lerp(v, dotBottomLeft, dotTopLeft),
                        Lerp(v, dotBottomRight, dotTopRight)
    );
    return result;
}