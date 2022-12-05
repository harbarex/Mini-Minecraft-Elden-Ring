#pragma once
#include "entity.h"
#include "camera.h"
#include "terrain.h"
#include "inventory.h"
#include "widget.h"
#include "blockinwidget.h"
#include "text.h"
#include <iostream>
#include <set>

enum class VelocityCond {stop, max, move};

class Player : public Entity {
private:
    glm::vec3 m_velocity, m_acceleration;
    Camera m_camera;
    Terrain &mcr_terrain;

    float flight_velocity_max, non_flight_velocity_max;
    float m_velocity_val, m_acceleration_val; // length of the vector
    float cameraBlockDist; // max distance from the camera while using ray tracing
    bool flightMode; // determine the current moving mode
    bool containerMode; // determine if the player opens the container or not
    double destroyBufferTime; // compute the passing time (s) starting from last destroy
    double creationBufferTime; // compute the passing time (s) starting from last block creation
    double minWaitTime; // the minimum waiting time (s) to destroy the next block

    void processInputs(InputBundle &inputs);
    void computePhysics(float dT, const Terrain &terrain, InputBundle &inputs);

    int selectedBlockOnHandPtr;
    Inventory inventory;
    Widget *inventoryWidgetOnHand;
    Widget *inventoryWidgetInContainer;
    BlockInWidget *inventoryItemOnHand;
    BlockInWidget *inventoryItemInContainer;
    Text* textOnScreen;

    bool checkXZCollision(int idx, const Terrain &terrain); // determine if current movement collide in X or Z axis (with idx 0 and 2)
    bool checkYCollision(const Terrain &terrain); // determine if current movement collide in Y axis (specifically for the ground)
    void implementJumping(const Terrain &terrain, InputBundle &inputs);
    void destroyBlock(InputBundle &inputs, Terrain &terrain); // destroy the block within 3 unit from camera pos when left mouse button is pressed
    void placeBlock(InputBundle &inputs, Terrain &terrain);

    // interaction in widget in container
    // the state indicating if the player is grabbing item or not
    bool isGrabbingItem;
    // the position where the player grab the item in container
    glm::vec2 grabItemScreenPos;
    // the overall index of the grabbed item
    glm::ivec2 grabItemOverallIdx;

    // set up the grab position in player object
    // called from MyGL
    void setGrabItemPos(float posX, float posY);

    // release the grab to given position in player object
    // called from MyGL
    void releaseGrabItem(float posX, float posY);

    // check if the player is grabbing item
    bool isGrabbing();
    // draw the grab item
    void widgetInteraction();


public:
    // Readonly public reference to our camera
    // for easy access from MyGL
    const Camera& mcr_camera;

    Player(glm::vec3 pos, Terrain &terrain);
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
    // for windows (main)
    void rotateCameraView(InputBundle &input);
    // for MacOS in the condition that QCursor::setPos does not work
    void rotateCameraView(float thetaChange, float phiChange);

    // check if any key associated with player's movement is pressed
    bool buttonIsPressed(InputBundle &inputs);
    // check if current player is moving or not (velocity)
    bool playerIsMoving(bool yCheck);
    // check the effect of acceleration on current velocity
    VelocityCond currVelocityCond(float dT, InputBundle &inputs);

    bool isOnGround(const Terrain &terrain, InputBundle &inputs);
    bool isUnderWater(const Terrain &terrain, InputBundle &inputs);
    bool isUnderLava(const Terrain &terrain, InputBundle &inputs);
    // check if the given position is liquid or not
    bool isLiquid(const Terrain &terrain, glm::ivec3* pos);

    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);
    bool gridMarchPrevBlock(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, glm::ivec3 *out_prevBlock, glm::ivec3 *out_blockHit);

    void setBlocksHold();
    void selectNextBlockOnHand(InputBundle &inputs);
    void selectBlockOnHand(InputBundle &inputs);

    void setupWidget(std::vector<Widget*> widgets);
    void setupText(Text* text);

    void drawInventoryItem();
    void drawInventoryItemOnHand();
    void drawInventoryItemInContainer();

    bool isOpenContainer();
    bool setContainerMode(bool state);
    bool toggleContainerMode();
};

