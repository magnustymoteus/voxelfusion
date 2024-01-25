//
// Created by daria on 27/11/2023.
//

#include "PerlinNoise.h"
#include <cmath>
#include <algorithm>
#include <random>

PerlinNoise::PerlinNoise(bool random){
    if(random){
        std::random_device dev;
        std::mt19937 rng(dev());
        std::vector<int> temporary(256);
        for(size_t i = 0; i != 256; i++){
            temporary[i] = i;
        }
        std::shuffle(temporary.begin(), temporary.end(), rng);
        temporary.resize(512);
        for(size_t i = 0; i != 256; i++){
            temporary[i+256] = temporary[i];
        }
        for(size_t i = 0; i != 512; i++){
            P[i] = temporary[i];
        }
    }
}

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
std::pair<double,double> PerlinNoise::getConstantVector(int v) {
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
std::pair<double,double> PerlinNoise::getPositiveConstantVector(int v) {
    // v is the value from the permutation table
    auto h = v & 3;
    if(h == 0)
        return std::pair(1.0, 1.0);
    else if(h == 1)
        return std::pair(0, 1.0);
    else if(h == 2)
        return  std::pair(0, 0);
    else
        return std::pair(1.0, 0);
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

    auto dotTopRight = topRight.first*getConstantVector(valueTopRight).first + topRight.second*getConstantVector(valueTopRight).second;
    auto dotTopLeft = topLeft.first*(getConstantVector(valueTopLeft)).first + topLeft.second*(getConstantVector(valueTopLeft)).second;
    auto dotBottomRight = bottomRight.first*(getConstantVector(valueBottomRight)).first + bottomRight.second*(getConstantVector(valueBottomRight)).second;
    auto dotBottomLeft = bottomLeft.first*(getConstantVector(valueBottomLeft)).first * bottomLeft.second*(getConstantVector(valueBottomLeft)).second;

    auto u = fade(xf);
    auto v = fade(yf);
    auto result = lerp(u,
                        lerp(v, dotBottomLeft, dotTopLeft),
                        lerp(v, dotBottomRight, dotTopRight)
    );
    return result;
}

double PerlinNoise::positiveNoise2d(const double& x, const double& y){
    const int X = static_cast<int>(std::floor(x)) & 255;
    const int Y = static_cast<int>(std::floor(y)) & 255;
    const double xf = x-std::floor(x);
    const double yf = y-std::floor(y);

    auto topRight = std::make_pair(1.0-xf, 1.0-yf);
    auto topLeft = std::make_pair(xf, 1.0-yf);
    auto bottomRight = std::make_pair(1.0-xf, yf);
    auto bottomLeft = std::make_pair(xf, yf);

    auto valueTopRight = P[P[X+1]+Y+1];
    auto valueTopLeft = P[P[X]+Y+1];
    auto valueBottomRight = P[P[X+1]+Y];
    auto valueBottomLeft = P[P[X]+Y];

    auto dotTopRight = topRight.first*getPositiveConstantVector(valueTopRight).first + topRight.second*getPositiveConstantVector(valueTopRight).second;
    auto dotTopLeft = topLeft.first*(getPositiveConstantVector(valueTopLeft)).first + topLeft.second*(getPositiveConstantVector(valueTopLeft)).second;
    auto dotBottomRight = bottomRight.first*(getPositiveConstantVector(valueBottomRight)).first + bottomRight.second*(getPositiveConstantVector(valueBottomRight)).second;
    auto dotBottomLeft = bottomLeft.first*(getPositiveConstantVector(valueBottomLeft)).first * bottomLeft.second*(getPositiveConstantVector(valueBottomLeft)).second;

    auto u = fade(xf);
    auto v = fade(yf);
    auto result = lerp(u,
                       lerp(v, dotBottomLeft, dotTopLeft),
                       lerp(v, dotBottomRight, dotTopRight)
    );
    return result;
}