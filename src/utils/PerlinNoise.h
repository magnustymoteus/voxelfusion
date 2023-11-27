//
// Created by daria on 27/11/2023.
//

#ifndef VOXELFUSION_PERLINNOISE_H
#define VOXELFUSION_PERLINNOISE_H


struct PerlinNoise {
    PerlinNoise(double noiseScale){scale = noiseScale;};
    double noise2d(const double& x, const double& y);
    double noise3d(const double& x, const double& y, const double& z);
    double scale;
};


#endif //VOXELFUSION_PERLINNOISE_H
