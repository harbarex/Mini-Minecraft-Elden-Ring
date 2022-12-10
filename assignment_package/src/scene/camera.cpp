#include "camera.h"
#include "glm_includes.h"

Camera::Camera(glm::vec3 pos)
    : Camera(400, 400, pos)
{}

Camera::Camera(unsigned int w, unsigned int h, glm::vec3 pos)
    : Entity(pos), m_fovy(45), m_width(w), m_height(h),
      m_near_clip(0.1f), m_far_clip(1000.f), m_aspect(w / static_cast<float>(h))
{}

Camera::Camera(const Camera &c)
    : Entity(c),
      m_fovy(c.m_fovy),
      m_width(c.m_width),
      m_height(c.m_height),
      m_near_clip(c.m_near_clip),
      m_far_clip(c.m_far_clip),
      m_aspect(c.m_aspect)
{}


void Camera::setWidthHeight(unsigned int w, unsigned int h) {
    m_width = w;
    m_height = h;
    m_aspect = w / static_cast<float>(h);
}


void Camera::tick(float dT, InputBundle &input) {
    // Do nothing
}

glm::mat4 Camera::getViewProj() const {
    return glm::perspective(glm::radians(m_fovy), m_aspect, m_near_clip, m_far_clip) * glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::vec3 Camera::getForward() {
    return m_forward;
}

glm::vec3 Camera::getRight() {
    return m_right;
}

glm::vec3 Camera::getUp() {
    return m_up;
}

glm::vec2 Camera::getScreenCenterPos() {
    return glm::vec2(m_width/2, m_height/2);
}

glm::vec3 Camera::getCurrentPos() {
    return m_position;
}


//---------------------------------------------
// Polar Spherical Camera Implementations
//---------------------------------------------

PSCamera::PSCamera(glm::vec3 pos, float zoom, float phi, float theta)
    : Camera(pos), zoom(zoom), phi(phi), theta(theta), worldUpDir(0.f, 1.f, 0.f)
{}

PSCamera::PSCamera(glm::vec3 pos)
    : PSCamera(pos, -8.f, 30.f, 190.f)
{}

void PSCamera::update(glm::vec3 pos)
{
    glm::mat4 rotTheta = glm::rotate(glm::mat4(1.f), glm::radians(theta), worldUpDir);
    glm::mat4 rotPhi = glm::rotate(glm::mat4(1.f), glm::radians(phi), glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 translation = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, zoom));

    // update this camera's forward, up, right vector
    m_forward = glm::normalize(glm::vec3(rotTheta * rotPhi * translation * glm::vec4(0, 0, 1, 0)));
    m_right = glm::normalize(glm::vec3(rotTheta * rotPhi * translation * glm::vec4(1, 0, 0, 0)));
    m_up = glm::normalize(glm::vec3(rotTheta * rotPhi * translation * glm::vec4(0, 1, 0, 0)));

    // update this camera's position
    m_position = glm::vec3(rotTheta * rotPhi * translation * glm::vec4(glm::vec3(0.f), 1)) + pos;

}
