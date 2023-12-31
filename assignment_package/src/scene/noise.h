#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <string>
#include <vector>

#define PI 3.14159265

class Noise{
public:
    Noise();
    virtual ~Noise();

    // General Function for Grass x Mountain x Waterbody
    int getHeight(int, int);

    float getCaveHeight(int, int, int);

    // Individual Terrain Height Maps
    float getGrassHeight(float, float);
    float getDirtHeight(float, float);
    float getRotLakeHeight(float, float);
    float getSandHeight(float, float);
    float getVolcanicRockHeight(float, float);
    float getAcidLakeHeight(float, float);
    float getLavaHeight(float, float);
    float getFloatingRockHeight(float, float);
    float getMountainousRockHeight(float,float);
    float getSnowHeight(float, float);
    float getSnowyRockHeight(float,float);
    float getWaterHeight(float,float);

    float getTreeProbability(float x, float z);

private:

    // Helper Functions

    // Grass
    const int firstOctave   = 2;
    const int octaves       = 8;
    const float persistence = 0.6;

    glm::vec2 getVoronoiCenter(glm::vec2);
    float worleyNoise2D(float,float);

    float FBM2D(float,float,float,std::string, int);
    float FBM3D(float,float,float,float,std::string);

    glm::vec2 noise2DNormalVector(glm::vec2, int);
    float surflet(glm::vec2, glm::vec2, int);
    float PerlinNoise2D(glm::vec2, int);

    float surflet3D(glm::vec3, glm::vec3);
    float PerlinNoise3D(glm::vec3);

    float noise2D(float, float);
    float smoothNoise2D(float,float);
    float linearInterpolation(float,float,float);
    float cosineInterpolation(float,float,float);
    float interpolationNoise2D(float,float);
    float interpolationNoise3D(float,float,float);
};





