#include "noise.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

Noise::Noise(){}

Noise::~Noise(){}

/**
 * @brief Noise::getHeight
 *
 * Get y coordinate with mixed terrain
 * @param x
 * @param z
 */
int Noise::getHeight(int x, int z) {
    float grass             = getGrassHeight(x, z);
    float mtn_rock          = getMountainousRockHeight(x, z);
    float water             = getWaterHeight(x, z);

    float perlin            = (FBM2D(x/2048.f, z/2048.f, 0.2, "perlin", 1) + 1) / 2;
    float smoothPerlin      = glm::smoothstep(0.5, 0.6, (double) perlin);

    float waterPerlin       = (FBM2D(x/4096.f, z/4096.f, 0.9, "perlin", 3) + 1) / 2;
    float waterSmoothPerlin = glm::smoothstep(0.8, 0.85, (double) waterPerlin);

    //return glm::clamp(glm::mix(glm::mix(grass, mtn_rock, smoothPerlin), water, waterSmoothPerlin), 128.f, 255.f);
    return glm::clamp(glm::mix(glm::mix(grass, water, waterSmoothPerlin), mtn_rock, smoothPerlin), 128.f, 255.f);
}

float Noise::getCaveHeight(int x, int y, int z){
    float factor = 25.f;
    return PerlinNoise3D(glm::vec3(float(x/factor),float(y/factor),float(z/factor)));

}

float Noise::getGrassHeight(float x, float z){
    x /= 512;
    z /= 512;

    int grassMin = 135;
    int grassMax = 142;

    return grassMin + (grassMax - grassMin) * worleyNoise2D(FBM2D(x, z, 0.5, "perlin", 1), FBM2D(z, x, 0.5, "perlin", 1));
}

float Noise::getMountainousRockHeight(float x, float z) {
    x /= 2048;
    z /= 2048;

    int mountainMin = 142;
    int mountainMax = 250;

    return mountainMin + (mountainMax - mountainMin) * glm::abs(FBM2D(x, z, 0.92, "perlin", 1));
}

float Noise::getWaterHeight(float x, float z){
    x /= 1024;
    z /= 1024;

    int waterMin = 128;
    int waterMax = 132;

    return waterMin + (waterMax - waterMin) * (FBM2D(x, z, 0.5, "perlin", 3) + 1) / 2;
}


float Noise::getSandHeight(float x, float z){
    x /= 2048;
    z /= 2048;

    int sandMin = 132;
    int sandMax = 135;

    return sandMin + (sandMax - sandMin) * (FBM2D(x, z, 0.5, "perlin", 3) + 1) / 2;
}


///////////////////////////////////////////////////
///////////////// Worley Noise ////////////////////
//////////////////////////////////////////////////


glm::vec2 Noise::getVoronoiCenter(glm::vec2 corner) {
    float x = glm::fract(glm::sin(glm::dot(corner, glm::vec2(127.1, 311.7))) * 43758.5453);
    float z = glm::fract(glm::sin(glm::dot(corner, glm::vec2(420.2, 1337.1))) * 789221.1234);
    return glm::vec2(x, z);
}

float Noise::worleyNoise2D(float x, float z) {
    float intX, fractX;
    fractX = modf(x, &intX);

    float intZ, fractZ;
    fractZ = modf(z, &intZ);

    float minDist1 = 1;
    float minDist2 = 1;

    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            glm::vec2 neighborDirection         = glm::vec2(j, i);
            glm::vec2 neighborVoronoiCenter     = getVoronoiCenter(glm::vec2(intX, intZ) + neighborDirection);
            glm::vec2 diff                      = neighborDirection + neighborVoronoiCenter - glm::vec2(fractX, fractZ);

            float dist = glm::length(diff);

            if (dist < minDist1) {
                minDist2 = minDist1;
                minDist1 = dist;
            } else if (dist < minDist2) {
                minDist2 = dist;
            }
        }
    }

    return minDist2 - minDist1;
}

//////////////////////////////////////////////////////////////
///////////////// Fractal Brownian Motion ////////////////////
/////////////////////////////////////////////////////////////

float Noise::FBM2D(float x, float z, float persistence, std::string noiseFn, int primeSet) {
    float total = 0;
    int octaves = 8;

    for (int i = 0; i < octaves; i++) {
        float frequency = pow(2, i);
        float amplitude = pow(persistence, i);

        if (noiseFn == "perlin") {
            total += PerlinNoise2D(glm::vec2(x * frequency, z * frequency), primeSet) * amplitude;
        }
        if (noiseFn == "regular") {
            total += interpolationNoise2D(x * frequency, z * frequency) * amplitude;
        }
    }

    return total;
}

float Noise::FBM3D(float x, float y, float z, float persistence, std::string noiseFn) {
    float total = 0;
    int octaves = 8;

    for (int i = 0; i < octaves; i++) {
        float frequency = pow(2, i);
        float amplitude = pow(persistence, i);

        if (noiseFn == "perlin") {
            total += PerlinNoise3D(glm::vec3(x * frequency, y * frequency, z * frequency)) * amplitude;
        }
//        if (noiseFn == "regular") {
//            total += interpolationNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
//        }
    }

    return total;
}


///////////////////////////////////////////////////
///////////////// Perlin Noise ////////////////////
//////////////////////////////////////////////////

/**
 * @brief pow
 *
 * Raise a given glm::vec2 or glm::vec3 to a particular power
 * @param v     : vector
 * @param power : exponent
 * @return
 */
glm::vec2 pow(glm::vec2 v, int power) {
    glm::vec2 p = v;
    for (int i = 0; i < power-1; i++) {
        p *= v;
    }
    return p;
}

glm::vec3 pow(glm::vec3 v, int power) {
    glm::vec3 p = v;
    for (int i = 0; i < power-1; i++) {
        p *= v;
    }
    return p;
}

float Noise::surflet(glm::vec2 p, glm::vec2 gridPoint, int primeSet) {
    glm::vec2 t2 = glm::abs(p - gridPoint);
    glm::vec2 t = glm::vec2(1.f) - 6.f * pow(t2, 5) + 15.f * pow(t2, 4) - 10.f * pow(t2, 3);

    glm::vec2 gradient = noise2DNormalVector(gridPoint, primeSet) * 2.f - glm::vec2(1,1);
    glm::vec2 diff = p - gridPoint;

    float height = glm::dot(diff, gradient);

    return height * t.x * t.y;
}

glm::vec3 random3(glm::vec3 c) {
    float j = 4096.0*sin(glm::dot(c,glm::vec3(17.0, 59.4, 15.0)));
    glm::vec3 r;
    r.z = glm::fract(512.0*j);
    j *= .125;
    r.x = glm::fract(512.0*j);
    j *= .125;
    r.y = glm::fract(512.0*j);
    return r - glm::vec3(0.5);
}


float Noise::surflet3D(glm::vec3 p, glm::vec3 gridPoint) {

    glm::vec3 t2    = glm::abs(p - gridPoint);
    glm::vec3 t     = glm::vec3(1.f) - 6.f * pow(t2, 5.f) + 15.f * pow(t2, 4.f) - 10.f * pow(t2, 3.f);

    glm::vec3 gradient  = glm::normalize(random3(gridPoint) * 2.f - glm::vec3(1.f));
    glm::vec3 diff      = p - gridPoint;

    float height = glm::dot(diff, gradient);

    return height * t.x * t.y * t.z;
}

float Noise::PerlinNoise2D(glm::vec2 p, int primeSet) {
    float surfletSum = 0.f;

    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            surfletSum += surflet(p, glm::floor(p) + glm::vec2(dx, dy), primeSet);
        }
    }

    return surfletSum;
}

float Noise::PerlinNoise3D(glm::vec3 p){
    float surfletSum = 0.f;

    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <=1; ++dz){
                surfletSum += surflet3D(p, glm::floor(p) + glm::vec3(dx, dy, dz));
            }
        }
    }

    return surfletSum;
}

//////////////////////////////////////////////////////////////
///////////////// Misc. Helpers /////////////////////////////
/////////////////////////////////////////////////////////////

glm::vec2 Noise::noise2DNormalVector(glm::vec2 v, int primeSet) {
    v += 0.1;

    float xMult;
    float yMult;
    glm::mat2 primes;

    if (primeSet == 1) {
        primes = glm::mat2{{126.1, 311.7}, {420.2, 1337.1}};
        xMult = 43758.5453;
        yMult = 789221.5453;
    }

    if (primeSet == 2) {
        primes = glm::mat2{{593.32, 931.85}, {719.31, 1029.44}};
        xMult = 354234.5048;
        yMult = 250986.2095;
    }

    if (primeSet == 3) {
        primes = glm::mat2{{958.11, 347.77}, {139.44, 9559.43}};
        xMult = 485048.09604;
        yMult = 9450.234234;
    }

    glm::vec2 noise = glm::sin(v * primes);
    noise.x *= xMult;
    noise.y *= yMult;

    return glm::normalize(glm::abs(glm::fract(noise)));
}

float Noise::linearInterpolation(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

float Noise::cosineInterpolation(float a, float b, float t) {
    t = (1 - cos(t * PI)) * 0.5;
    return linearInterpolation(a, b, t);
}

float Noise::interpolationNoise2D(float x, float z) {
    float intX, fractX;
    fractX = modf(x, &intX);

    float intZ, fractZ;
    fractZ = modf(z, &intZ);

    float v1 = smoothNoise2D(intX, intZ);
    float v2 = smoothNoise2D(intX + 1, intZ);
    float v3 = smoothNoise2D(intX, intZ + 1);
    float v4 = smoothNoise2D(intX + 1, intZ + 1);

    float i1 = cosineInterpolation(v1, v2, fractX);
    float i2 = cosineInterpolation(v3, v4, fractX);

    return cosineInterpolation(i1, i2, fractZ);
}

float Noise::smoothNoise2D(float x, float z) {
    float corners = (noise2D(x - 1, z - 1) +
                     noise2D(x + 1, z - 1) +
                     noise2D(x - 1, z + 1) +
                     noise2D(x + 1, z + 1)) / 16;
    float sides = (noise2D(x - 1, z) +
                   noise2D(x + 1, z) +
                   noise2D(x, z - 1) +
                   noise2D(x, z + 1)) / 8;
    float center = noise2D(x, z) / 4;

    return corners + sides + center;
}

float Noise::noise2D(float x, float z) {
    float s = sin(glm::dot(glm::vec2(x, z), glm::vec2(127.1, 311.7))) * 43758.5453;
    return modf(s, nullptr);
}
