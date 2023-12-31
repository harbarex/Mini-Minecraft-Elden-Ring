#pragma once
#include "glm_includes.h"

struct InputBundle {
    bool wPressed, aPressed, sPressed, dPressed;
    // flight mode is active
    bool qPressed, ePressed;
    // flight mode is inactive
    bool spacePressed;
    bool leftMouseButtonPressed, rightMouseButtonPressed;
    bool pPressed;
    bool nPressed;
    bool iPressed;
    float mouseX, mouseY;

    // Checking Terrain
    bool underWater;
    bool underLava;
    bool onGround;

    // select the block (from 0 to 9, but 0 is not used)
    bool numberPressed[10];

    InputBundle()
        : wPressed(false), aPressed(false), sPressed(false),
          dPressed(false), qPressed(false), ePressed(false), spacePressed(false),
          leftMouseButtonPressed(false), rightMouseButtonPressed(false), pPressed(false),
          nPressed(false), iPressed(false), mouseX(0.f), mouseY(0.f), underWater(false), underLava(false),
          onGround(false), numberPressed {false, false, false, false, false, false, false, false, false, false}
    {}
};

class Entity {
protected:
    // Vectors that define the axes of our local coordinate system
    glm::vec3 m_forward, m_right, m_up;
    // The origin of our local coordinate system
    glm::vec3 m_position;

public:
    // A readonly reference to position for external use
    const glm::vec3& mcr_position;

    // Various constructors
    Entity();
    Entity(glm::vec3 pos);
    Entity(const Entity &e);
    virtual ~Entity();

    // To be called by MyGL::tick()
    virtual void tick(float dT, InputBundle &input) = 0;

    // Translate along the given vector
    virtual void moveAlongVector(glm::vec3 dir);

    // Translate along one of our local axes
    virtual void moveForwardLocal(float amount);
    virtual void moveRightLocal(float amount);
    virtual void moveUpLocal(float amount);

    // Translate along one of the world axes
    virtual void moveForwardGlobal(float amount);
    virtual void moveRightGlobal(float amount);
    virtual void moveUpGlobal(float amount);

    // Rotate about one of our local axes
    virtual void rotateOnForwardLocal(float degrees);
    virtual void rotateOnRightLocal(float degrees);
    virtual void rotateOnUpLocal(float degrees);

    // Rotate about one of the world axes
    virtual void rotateOnForwardGlobal(float degrees);
    virtual void rotateOnRightGlobal(float degrees);
    virtual void rotateOnUpGlobal(float degrees);
};
