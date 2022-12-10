#pragma once
#include "glm_includes.h"
#include "scene/entity.h"

//A perspective projection camera
//Receives its eye position and reference point from the scene XML file
class Camera : public Entity {
private:
    float m_fovy;
    unsigned int m_width, m_height;  // Screen dimensions
    float m_near_clip;  // Near clip plane distance
    float m_far_clip;  // Far clip plane distance
    float m_aspect;    // Aspect ratio

public:
    Camera(glm::vec3 pos);
    Camera(unsigned int w, unsigned int h, glm::vec3 pos);
    Camera(const Camera &c);
    void setWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    glm::mat4 getViewProj() const;

    // get current camera orientation
    glm::vec3 getForward();
    glm::vec3 getRight();
    glm::vec3 getUp();

    // get center position of the screen
    glm::vec2 getScreenCenterPos();
    // get current camera position
    glm::vec3 getCurrentPos();
};

// Polar Spherical Camera
class PSCamera : public Camera
{
private:

    // use these three params to set up forward, right & up vector
    float zoom, phi, theta;

    glm::vec3 worldUpDir;

public:

    PSCamera(glm::vec3 pos, float zoom, float phi, float theta);
    PSCamera(glm::vec3 pos);

    // update the PSCamera based on the spherical center
    // currently, this must be the player camera's position
    void update(glm::vec3 pos);
};
