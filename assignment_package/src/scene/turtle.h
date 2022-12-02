#ifndef TURTLE_H
#define TURTLE_H

#pragma once

#include <glm/common.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

using namespace glm;
using namespace std;

const float PI = 3.141592653589793238463;

class Turtle {
public:
    vec2    m_position;
    vec2    m_orientation;
    float   m_distance;
    int     m_depth;

    Turtle();
    Turtle(vec2 position, vec2 orientation, float distance);
    Turtle(const Turtle& t);

    void rotateRight(const float factor);
    void rotateLeft(const float factor);
    void moveForward();
};


#endif // TURTLE_H
