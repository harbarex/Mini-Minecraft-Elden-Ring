#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include <iostream>

enum class VelocityCond {stop, max, move};

class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    const Terrain &mcr_terrain;

    float m_velocity_val, m_acceleration_val; // length of the vector
    bool flightMode; // determine the current mode

    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain, InputBundle &inputs);

public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;

    Player(glm::vec3 pos, const Terrain &terrain);
    virtual ~Player() override;

    void setCameraWidthHeight(unsigned int w, unsigned int h);

    void tick(float dT, InputBundle &input) override;

    // Player overrides all of Entity's movement
    // functions so that it transforms its camera
    // by the same amount as it transforms itself.
    void moveAlongVector(glm::vec3 dir) override;
    void moveForwardLocal(float amount) override;
    void moveRightLocal(float amount) override;
    void moveUpLocal(float amount) override;
    void moveForwardGlobal(float amount) override;
    void moveRightGlobal(float amount) override;
    void moveUpGlobal(float amount) override;
    void rotateOnForwardLocal(float degrees) override;
    void rotateOnRightLocal(float degrees) override;
    void rotateOnUpLocal(float degrees) override;
    void rotateOnForwardGlobal(float degrees) override;
    void rotateOnRightGlobal(float degrees) override;
    void rotateOnUpGlobal(float degrees) override;

    // For sending the Player's data to the GUI
    // for display
    QString posAsQString() const;
    QString velAsQString() const;
    QString accAsQString() const;
    QString lookAsQString() const;

    // toggle current flight mode
    void toggleFlightMode();

    // rotate camera view based on the position of the cursor
//    void rotateCameraView(InputBundle &input);
    void rotateCameraView(float thetaChange, float phiChange);

    // check if any key associated with player's movement is pressed
    bool buttonIsPressed(InputBundle &inputs);
    // check if current player is moving or not (velocity)
    bool playerIsMoving();
    // check the effect of acceleration on current velocity
    VelocityCond currVelocityCond(float dT, InputBundle &inputs);

};

