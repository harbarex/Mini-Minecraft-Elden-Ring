#include "mygl.h"
#include "scene/npcs/sheep.h"

#include "scene/npcs/zombiedragon.h"
#include "scene/npcs/lama.h"
#include <glm_includes.h>
#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QFile>
#include <algorithm>
#include <random>



MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this),
      m_progUnderwater(this), m_progLava(this), m_progNoOp(this), m_progInventoryWidgetOnHand(this), m_progInventoryItemOnHand(this), m_progInventoryWidgetInContainer(this),
      m_progInventoryItemInContainer(this), m_progGrabbedItem(this), m_progText(this),
      m_quad(this), m_progNPC(this), m_frameBuffer(this, this->width(), this->height(), this->devicePixelRatio()),
      m_terrain(this), m_player(glm::vec3(48.f, 200.f, 48.f), m_terrain),
      m_player_model(this, glm::vec3(60.f, 145.f, 35.f), m_terrain, m_player, STEVE), frameCount(0),
      prevFrameTime(QDateTime::currentMSecsSinceEpoch()), mouseCursorMode(false), textureAll(this), inventoryWidgetOnHandTexture(this), inventoryWidgetInContainerTexture(this),
      textureFont(this), prevExpandTime(QDateTime::currentMSecsSinceEpoch())
{

    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);
    m_inputs = InputBundle();
    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
//    setCursor(Qt::BlankCursor); // Make the cursor invisible
    setCursor(Qt::CrossCursor);
    prevMouseX = width() / 2;
    prevMouseY = height() / 2;

    setupNPCs();

    initWidget();
    initText();

}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_quad.destroyVBOdata();
    inventoryWidgetOnHand->destroyVBOdata();
    inventoryItemsOnHand->destroyVBOdata();
    textOnScreen->destroyVBOdata();
    m_frameBuffer.destroy();
    m_worldAxes.destroyVBOdata();
}


void MyGL::moveMouseToCenter() {
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // The order DOES MATTER
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    // Create the instance of the post-process quad
    m_quad.createVBOdata();

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Initiailize frame buffer
    m_frameBuffer.create();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
//    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

    m_progUnderwater.create(":/glsl/post/overlay.vert.glsl", ":/glsl/post/underwater.frag.glsl");
    m_progLava.create(":/glsl/post/overlay.vert.glsl", ":/glsl/post/lava.frag.glsl");
    m_progNoOp.create(":/glsl/post/overlay.vert.glsl", ":/glsl/post/overlay.frag.glsl");
    m_progInventoryWidgetOnHand.create(":/glsl/post/texture.vert.glsl", ":/glsl/post/texture.frag.glsl");
    m_progInventoryItemOnHand.create(":/glsl/post/texture.vert.glsl", ":/glsl/post/texture.frag.glsl");
    m_progInventoryWidgetInContainer.create(":/glsl/post/texture.vert.glsl", ":/glsl/post/texture.frag.glsl");
    m_progInventoryItemInContainer.create(":/glsl/post/texture.vert.glsl", ":/glsl/post/texture.frag.glsl");
    m_progGrabbedItem.create(":/glsl/post/texture.vert.glsl", ":/glsl/post/texture.frag.glsl");
    m_progText.create(":/glsl/post/texture.vert.glsl", ":/glsl/post/texture.frag.glsl");


    m_progNPC.create(":/glsl/lambert.vert.glsl", ":/glsl/npc.frag.glsl");

    m_quad.createVBOdata();

    createNPCTextures();

    // initialize Steve
    m_player_model.createVBOdata();
    m_player_model.initSceneGraph();

    // create NPC's VBO and initialize NPC scene graph
    for (const uPtr<NPC> &npc : m_npcs)
    {
        npc->createVBOdata();
        npc->initSceneGraph();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    /// loading texture map from png
    ////////////////////////////////////////////////////////////////////////////////////
    // main texture map (slot = 0)
    createTexture(textureAll, ":/textures/minecraft_textures_all.png", 0);
    // loading uv coordinate of main texture map from text file
    Block::loadUVCoordFromText(":/textures/uv_coord_texture_all.txt");

    // widget texture map (slot = 2)
    createTexture(inventoryWidgetOnHandTexture, ":/textures/minecraft_textures_widgets.png", 2);
    inventoryWidgetOnHand->loadCoordFromText(":/textures/widget_on_hand_info.txt");

    // block in widget (on hand)
    inventoryItemsOnHand->loadCoordFromText(":/textures/widget_item_on_hand_info.txt");

    // container widget texture map (slot = 4)
    createTexture(inventoryWidgetInContainerTexture, ":/textures/inventory.png", 4);
    inventoryWidgetInContainer->loadCoordFromText(":/textures/widget_in_container_info.txt");

    // item in widget in container
    inventoryItemsInContainer->loadCoordFromText(":/textures/widget_item_in_container_info.txt");

    // text on the screen (slot = 3)
    createTexture(textureFont, ":/textures/ascii.png", 3);
    textOnScreen->loadUVCoordFromText(":/textures/text_info.txt");

    ////////////////////////////////////////////////////////////////////////////////////

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));
    m_progNPC.setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
    glBindVertexArray(vao);

    // m_terrain.CreateTestScene();
    // m_terrain.CreateTestGrassScene();
}

void MyGL::resizeGL(int w, int h) {
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    m_player.setCameraWidthHeight(static_cast<unsigned int>(w), static_cast<unsigned int>(h));
    glm::mat4 viewproj = m_player.getCameraViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progUnderwater.setViewProjMatrix(viewproj);

    m_frameBuffer.resize(this->width(), this->height(), this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();
    m_progNPC.setViewProjMatrix(viewproj);

    m_progNoOp.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    m_progUnderwater.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    m_progLava.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    m_progInventoryWidgetOnHand.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));

    m_frameBuffer.resize(this->width(), this->height(), this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();

    textOnScreen->resizeDimension(this->width(), this->height());

    printGLErrorLog();
}


// MyGL's constructor links tick() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to perform
// all per-frame actions here, such as performing physics updates on all
// entities in the scene.
void MyGL::tick() {
    update(); // Calls paintGL() as part of a larger QOpenGLWidget pipeline
    sendPlayerDataToGUI(); // Updates the info in the secondary window displaying player data

    // call terrain expansion
    // TODO: use 5 x 5 zones
    if (!m_terrain.m_initialTerrainLoaded) {
        m_terrain.loadInitialTerrain(m_player.mcr_position[0], m_player.mcr_position[2], 2);
        prevExpandTime = QDateTime::currentMSecsSinceEpoch();
    }
    else if ((QDateTime::currentMSecsSinceEpoch() - prevExpandTime) >= 100)
    {
        m_terrain.expand(m_player.mcr_position[0], m_player.mcr_position[2], 2);
        prevExpandTime = QDateTime::currentMSecsSinceEpoch();
    }
    // check & (draw) send to gpu
    m_terrain.checkThreadResults();

    // compute the delta-time
    long long currFrameTime = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = (currFrameTime - prevFrameTime) / 1000.f;

    prevFrameTime = currFrameTime;

    drawGrabbedItem();

    // pass delta-time to Player::tick
    m_player.tick(deltaTime, m_inputs);
    // steve model
    m_player_model.tick(deltaTime, m_inputs);

    // TODO: pass delta-time to NPC's tick as well
    if (frameCount > 15.f * 60.f)
    {
        for (const uPtr<NPC> &npc : m_npcs)
        {
            npc->tick(deltaTime);
        }
    }

}

void MyGL::sendPlayerDataToGUI() const {
    emit sig_sendPlayerPos(m_player.posAsQString());
    emit sig_sendPlayerVel(m_player.velAsQString());
    emit sig_sendPlayerAcc(m_player.accAsQString());
    emit sig_sendPlayerLook(m_player.lookAsQString());
    glm::vec2 pPos(m_player.mcr_position.x, m_player.mcr_position.z);
    glm::ivec2 chunk(16 * glm::ivec2(glm::floor(pPos / 16.f)));
    glm::ivec2 zone(64 * glm::ivec2(glm::floor(pPos / 64.f)));
    emit sig_sendPlayerChunk(QString::fromStdString("( " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + " )"));
    emit sig_sendPlayerTerrainZone(QString::fromStdString("( " + std::to_string(zone.x) + ", " + std::to_string(zone.y) + " )"));
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL() {
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind FrameBuffer for Overlay
    m_frameBuffer.bindFrameBuffer();
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_progFlat.setViewProjMatrix(m_player.getCameraViewProj());
    m_progLambert.setViewProjMatrix(m_player.getCameraViewProj());
    m_progNPC.setViewProjMatrix(m_player.getCameraViewProj());

    m_progLambert.setTime(frameCount);
    m_progLava.setTime(frameCount);
    m_progUnderwater.setTime(frameCount);
    m_progNPC.setTime(frameCount);

    renderTerrain(TerrainDrawType::opaque);

    glDisable(GL_DEPTH_TEST);

    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.getCameraViewProj());

    //m_progFlat.draw(m_worldAxes);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderTerrain(TerrainDrawType::transparent);
    // render steve
    renderPlayerModel();
    // render NPCs
    renderNPCs();
    glDisable(GL_BLEND);

    // Draw Golden Tree (s)
    m_terrain.drawErdtree(glm::ivec2(32, 48));



    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_frameBuffer.bindToTextureSlot(1);

    // Post-process Shaders
    if(m_player.isUnderWater(m_terrain, m_inputs)){
        m_progUnderwater.setTexture(m_frameBuffer.getTextureSlot());
        m_progUnderwater.drawOverlay(m_quad);
    }
    else if(m_player.isUnderLava(m_terrain, m_inputs)){
        m_progLava.setTexture(m_frameBuffer.getTextureSlot());
        m_progLava.drawOverlay(m_quad);
    }
    else{
        m_progNoOp.setTexture(m_frameBuffer.getTextureSlot());
        m_progNoOp.drawOverlay(m_quad);
    }

    // draw the widget at last
    glDisable(GL_DEPTH_TEST);
    // On hand
    // widget and selected frame
    renderTexture(inventoryWidgetOnHandTexture, m_progInventoryWidgetOnHand, 2, inventoryWidgetOnHand);
    // blocks
    renderTexture(textureAll, m_progInventoryItemOnHand, 0, inventoryItemsOnHand);
    // In container
    // TODO: draw the container widget and items in container if the player opens it
    if (m_player.isOpenContainer()) {
        renderTexture(inventoryWidgetInContainerTexture, m_progInventoryWidgetInContainer, 4, inventoryWidgetInContainer);
        renderTexture(textureAll, m_progInventoryItemInContainer, 0, inventoryItemsInContainer);
        if (m_player.isGrabbing()) {
            renderTexture(textureAll, m_progGrabbedItem, 0, grabbedItem);
        }
    }

    glEnable(GL_BLEND);
    renderTexture(textureFont, m_progText, 3, textOnScreen.get());
    glDisable(GL_BLEND);

    glEnable(GL_DEPTH_TEST);


    frameCount++;
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain(TerrainDrawType drawType) {

    // bind the texture
    bindTexture(textureAll, m_progLambert, 0);

    // only draw the 3 x 3 chunks around the player
    glm::vec3 pos = m_player.mcr_position;
    m_terrain.draw(pos[0], pos[2], 2, &m_progLambert, drawType);
}


void MyGL::keyPressEvent(QKeyEvent *e) {
    float amount = 2.0f;
    if(e->modifiers() & Qt::ShiftModifier){
        amount = 10.0f;
    }
    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    // m_inputs = InputBundle();

    if (e->key() == Qt::Key_Escape) {
        // clear the thread pool and join the running threads
        QThreadPool::globalInstance()->clear();
        QThreadPool::globalInstance()->waitForDone(-1);
        QApplication::quit();
    } else if (e->key() == Qt::Key_Right) {
        m_player.rotateOnUpGlobal(-amount);
    } else if (e->key() == Qt::Key_Left) {
        m_player.rotateOnUpGlobal(amount);
    } else if (e->key() == Qt::Key_Up) {
        m_player.rotateOnRightLocal(-amount);
    } else if (e->key() == Qt::Key_Down) {
        m_player.rotateOnRightLocal(amount);
    } else if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = true;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = true;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = true;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = true;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = true;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = true;
    } else if (e->key() == Qt::Key_P) {
        m_inputs.pPressed = true;
        m_player.fillAllBlocks();
    } else if (e->key() == Qt::Key_N) {
        m_inputs.nPressed = true;
        m_player.selectNextBlockOnHand(m_inputs);
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_player.toggleFlightMode();
    } else if (e->key() == Qt::Key_0) {
        m_inputs.numberPressed[0] = true;
    } else if (e->key() == Qt::Key_1) {
        m_inputs.numberPressed[1] = true;
    } else if (e->key() == Qt::Key_2) {
        m_inputs.numberPressed[2] = true;
    } else if (e->key() == Qt::Key_3) {
        m_inputs.numberPressed[3] = true;
    } else if (e->key() == Qt::Key_4) {
        m_inputs.numberPressed[4] = true;
    } else if (e->key() == Qt::Key_5) {
        m_inputs.numberPressed[5] = true;
    } else if (e->key() == Qt::Key_6) {
        m_inputs.numberPressed[6] = true;
    } else if (e->key() == Qt::Key_7) {
        m_inputs.numberPressed[7] = true;
    } else if (e->key() == Qt::Key_8) {
        m_inputs.numberPressed[8] = true;
    } else if (e->key() == Qt::Key_9) {
        m_inputs.numberPressed[9] = true;
    } else if (e->key() == Qt::Key_I) {
        m_inputs.iPressed = true;
    } else if (e->key() == Qt::Key_C) {
        m_player.switchCameraView();
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_W) {
        m_inputs.wPressed = false;
    } else if (e->key() == Qt::Key_S) {
        m_inputs.sPressed = false;
    } else if (e->key() == Qt::Key_D) {
        m_inputs.dPressed = false;
    } else if (e->key() == Qt::Key_A) {
        m_inputs.aPressed = false;
    } else if (e->key() == Qt::Key_Q) {
        m_inputs.qPressed = false;
    } else if (e->key() == Qt::Key_E) {
        m_inputs.ePressed = false;
    } else if (e->key() == Qt::Key_P) {
        m_inputs.pPressed = false;
    } else if (e->key() == Qt::Key_N) {
        m_inputs.nPressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
    } else if (e->key() == Qt::Key_0) {
        m_inputs.numberPressed[0] = false;
    } else if (e->key() == Qt::Key_1) {
        m_inputs.numberPressed[1] = false;
    } else if (e->key() == Qt::Key_2) {
        m_inputs.numberPressed[2] = false;
    } else if (e->key() == Qt::Key_3) {
        m_inputs.numberPressed[3] = false;
    } else if (e->key() == Qt::Key_4) {
        m_inputs.numberPressed[4] = false;
    } else if (e->key() == Qt::Key_5) {
        m_inputs.numberPressed[5] = false;
    } else if (e->key() == Qt::Key_6) {
        m_inputs.numberPressed[6] = false;
    } else if (e->key() == Qt::Key_7) {
        m_inputs.numberPressed[7] = false;
    } else if (e->key() == Qt::Key_8) {
        m_inputs.numberPressed[8] = false;
    } else if (e->key() == Qt::Key_9) {
        m_inputs.numberPressed[9] = false;
    } else if (e->key() == Qt::Key_I) {
        m_inputs.iPressed = false;
        m_player.toggleContainerMode();
        toggleMouseCursorMode();
    }
}

void MyGL::mouseMoveEvent(QMouseEvent *e) {
    // TODO
    m_inputs.mouseX = e->pos().x();
    m_inputs.mouseY = e->pos().y();

//    #ifdef __APPLE__
//    // for MacOS, but the cursor still cannot keep in center
//    m_player.rotateCameraView(m_inputs.mouseX - prevMouseX, m_inputs.mouseY - prevMouseY);
//    prevMouseX = m_inputs.mouseX;
//    prevMouseY = m_inputs.mouseY;
//    #elif _WIN32
//    // for windows
//    m_player.rotateCameraView(m_inputs);

//    #endif

    if (mouseCursorMode) {
        return;
    }
    m_player.rotateCameraView(m_inputs);
    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {

    switch (e->button()) {
    case (Qt::LeftButton):
        m_inputs.leftMouseButtonPressed = true;
        // TODO: grab the block item if the player opens the container
        if (!m_player.isGrabbing() && m_player.isOpenContainer()) {
            glm::vec2 pos = convertPosToNormalizedPos(e);
            if (m_player.setGrabItemPos(pos.x, pos.y)){
                grabbedItemType = m_player.getGrabbedItemType();
                Block::getUVCoords(grabbedItemType, &grabbedItemUVCoords);
            }
        }
        break;
    case (Qt::RightButton):
        m_inputs.rightMouseButtonPressed = true;
        break;
    default:
        return;
    }
}

void MyGL::mouseReleaseEvent(QMouseEvent *e) {

    switch (e->button()) {
    case (Qt::LeftButton):
        m_inputs.leftMouseButtonPressed = false;
        // TODO: release the selected block item to a new place (or switch the items)
        // if the player opens the container
        if (m_player.isGrabbing() && m_player.isOpenContainer()) {
            glm::vec2 pos = convertPosToNormalizedPos(e);
            m_player.releaseGrabItem(pos.x, pos.y);
        }
        break;
    case (Qt::RightButton):
        m_inputs.rightMouseButtonPressed = false;
        break;
    default:
        return;
    }
}

/**
 * @brief MyGL::createTexture
 *   assign texture image to the texture object
 *   called from initializeGL
 * @param texture :texture object that the texture map would assign to
 * @param img_path : image path of texture map (in qrc)
 * @param slot: slot that would be allocated to this texture object
 */
void MyGL::createTexture(Texture& texture, const char* img_path, int slot) {
    texture.create(img_path);
    texture.load(slot);
}


/**
 * @brief MyGL::createNPCTextures
 *  Register the NPC textures
 */
void MyGL::createNPCTextures()
{
    loadNPCTextureUVCoord();
    // Steve
    npcTextures[STEVE] = Texture(this);
    npcTextures[STEVE].create(":/textures/steve.png");
    npcTextures[STEVE].load(3);

    // Sheep
    npcTextures[SHEEP] = Texture(this);
    npcTextures[SHEEP].create(":/textures/sheep.png");
    npcTextures[SHEEP].load(4);

    // ZombieDragon
    npcTextures[ZDRAGON] = Texture(this);
    npcTextures[ZDRAGON].create(":/textures/zdragon.png");
    npcTextures[ZDRAGON].load(5);

    // Lama
    npcTextures[GLAMA] = Texture(this);
    npcTextures[GLAMA].create(":/textures/graylama.png");
    npcTextures[GLAMA].load(6);
    npcTextures[WLAMA] = Texture(this);
    npcTextures[WLAMA].create(":/textures/whitelama.png");
    npcTextures[WLAMA].load(7);
    npcTextures[BLAMA] = Texture(this);
    npcTextures[BLAMA].create(":/textures/brownlama.png");
    npcTextures[BLAMA].load(8);

    // TODO: others
}


/**
 * @brief MyGL::bindTexture
 *   helper function to bind the shadeProgram to texture
 * @param texture : texture object
 * @param shaderProgram : shaderProgram
 * @param slot: texture slot
 */
void MyGL::bindTexture(Texture& texture, ShaderProgram& shaderProgram, int slot) {
    texture.bind(slot);
    shaderProgram.setTexture(slot);
}


/**
 * @brief MyGL::renderTexture
 *   render the object only requires the texture map
 *   called from paintGL
 * @param texture : texture object used in this drawable object
 * @param shaderProgram : shaderProgram of the drawable object
 * @param slot: texture slot
 * @param drawable : pointer to drawable object (for inherited class)
 */
void MyGL::renderTexture(Texture& texture, ShaderProgram& shaderProgram, int slot, Drawable* d) {
    d->createVBOdata();
    bindTexture(texture, shaderProgram, slot);
    shaderProgram.drawTexture(*d);
}

void MyGL::initWidget() {
    /**
     * widget setup
     * 1. inventoryWidgetOnHand
     * 2. inventoryItemOnHand
     * 3. inventoryWidgetInContainer
     * 4. inventoryItemInContainer
     */
    std::vector<Widget*> widgets_raw;

    uPtr<Widget> widget = mkU<Widget>(this);
    inventoryWidgetOnHand = widget.get();
    widgets_raw.push_back(inventoryWidgetOnHand);
    widgets.push_back(std::move(widget));

    uPtr<BlockInWidget> blockInWidget = mkU<BlockInWidget>(this);
    inventoryItemsOnHand = blockInWidget.get();
    widgets_raw.push_back(inventoryItemsOnHand);
    widgets.push_back(std::move(blockInWidget));

    uPtr<Widget> widget2 = mkU<Widget>(this);
    inventoryWidgetInContainer = widget2.get();
    widgets_raw.push_back(inventoryWidgetInContainer);
    widgets.push_back(std::move(widget2));

    uPtr<BlockInWidget> blockInWidget2 = mkU<BlockInWidget>(this);
    inventoryItemsInContainer = blockInWidget2.get();
    widgets_raw.push_back(inventoryItemsInContainer);
    widgets.push_back(std::move(blockInWidget2));

    uPtr<BlockInWidget> blockInWidget3 = mkU<BlockInWidget>(this);
    grabbedItem = blockInWidget3.get();
    widgets_raw.push_back(grabbedItem);
    widgets.push_back(std::move(blockInWidget3));

    // pass widget raw pointers to player
    m_player.setupWidget(widgets_raw);
}

void MyGL::initText() {
    textOnScreen = mkU<Text>(this, width(), height());
    m_player.setupText(textOnScreen.get());

}

void MyGL::toggleMouseCursorMode() {
    if (mouseCursorMode) {
        mouseCursorMode = false;
    } else {
        mouseCursorMode = true;
    }
}

/**
 * @brief MyGL::convertPosToNormalizedPos
 *  helper function to normalize the position
 * @param pos : float, the position where the mouse click
 * ranging from 0 to the width and height for x and y, respectively
 * @return normalizePos : glm::vec2, the normalized position where the mouse click
 * ranging from -1 to 1 for x and y, respectively
 */
glm::vec2 MyGL::convertPosToNormalizedPos(QMouseEvent *e) {
    // TODO: finish the function
    float halfWidth = width()/2.f;
    float halfHeight = height()/2.f;
    float posX = (e->pos().x() - halfWidth) / halfWidth;
    float posY = (halfHeight - e->pos().y()) / halfHeight;
    return glm::vec2(posX, posY);
}

glm::vec2 MyGL::convertPosToNormalizedPos(glm::vec2 pixelPos) {
    float halfWidth = width()/2.f;
    float halfHeight = height()/2.f;
    float posX = (pixelPos.x - halfWidth) / halfWidth;
    float posY = (halfHeight - pixelPos.y) / halfHeight;
    return glm::vec2(posX, posY);
}

void MyGL::drawGrabbedItem() {
    // draw the grabbed item
    if (!m_player.isGrabbing() || !m_player.isOpenContainer()) {
        return;
    }
    QPoint point = QWidget::mapFromGlobal(QCursor::pos());
    glm::vec2 mousePos(point.x(), point.y());

    glm::vec2 pos = convertPosToNormalizedPos(mousePos);
    // hard-code
    glm::vec2 len(0.0945, 0.130);
    grabbedItem->addItem(pos, len, grabbedItemUVCoords);
}


void MyGL::loadNPCTextureUVCoord()
{
    // read text file
    QFile f(":/textures/npc_uv_coord.txt");
    f.open(QIODevice::ReadOnly);
    QTextStream s(&f);
    QString line;
    bool readCoord = false;
    int coordCount = 0;
    std::array<glm::vec4, 6> uvs;
    BlockType currBlockType;

    while (!s.atEnd()) {
        line = s.readLine();
        if (line.size() == 0 || line.startsWith("#")) {
            // skip empty line and line starts with #
            continue;
        }
        if (readCoord) {
            // store uv coordinate into temp array
            QStringList items = line.split(" ");
            uvs[coordCount] = glm::vec4(items[0].toDouble(), items[1].toDouble(), items[2].toDouble(), items[3].toDouble());
            coordCount += 1;
        } else if (Block::blockTypeMap.find(line.toStdString()) != Block::blockTypeMap.end()) {
            // identify which block
            currBlockType = Block::blockTypeMap[line.toStdString()];
            readCoord = true;
        }

        if (coordCount == 6) {
            // store 6 uv coordinates into BlockCollection
            Block::insertNewUVCoord(currBlockType, uvs);
            uvs.empty();
            coordCount = 0;
            readCoord = false;
        }
    }
}

/**
 * @brief MyGL::renderNPCs
 *  The main logics to draw all the NPCs in paintGL().
 *  This helper is called in MyGL::paintGL().
 *  Basically, loop through the m_npcs.
 *  For each npc, traverse the scene graph,
 *  set the model matrix based on the node's transformation,
 *  draw the corresponding block.
 *  Note: the scene graph / the blocks of every NPC must be created.
 */
void MyGL::renderNPCs()
{
    for (const uPtr<NPC> &npc : m_npcs)
    {
        // Retrieve the texture map
        if (npcTextures.find(npc->npcTexture) == npcTextures.end())
        {
            continue;
        }
        // bind the texture map
        npcTextures[npc->npcTexture].bind(npcTextures[npc->npcTexture].getSlot());
        m_progNPC.setTexture(npcTextures[npc->npcTexture].getSlot());
        npc->draw(&m_progNPC);
    }
}

/**
 * @brief MyGL::renderPlayerModel
 *  Render Steve
 */
void MyGL::renderPlayerModel()
{
    if (npcTextures.find(m_player_model.npcTexture) != npcTextures.end())
    {
        npcTextures[m_player_model.npcTexture].bind(npcTextures[m_player_model.npcTexture].getSlot());
        m_progNPC.setTexture(npcTextures[m_player_model.npcTexture].getSlot());
        m_player_model.draw(&m_progNPC);
    }
}

/**
 * @brief MyGL::setupNPCs
 *  This helper contains the initial setup of all NPCs in this world.
 *  -------------
 *  General Steps to setup a NPC:
 *  0. (Before instantiate a NPC here)
 *      - add NPCTexture in texture.h if it's a new NPC
 *      - create the corresponding NPC's texture in MyGL::createNPCTextures()
 *  1. Instantiate an NPC entity (set up parameters)
 *      - params
 *          - initial position
 *          - (a series of) goals to achieve
 *          - initial movement speed (in x & z)
 *          - tolerance of distance to determine the achievement of a goal
 *          - tolerance of distance to determine the completion of a step
 *          - the search grid (halfGridSize) of the path finder (A* search)
 *      - other needed params (m_terrain, m_player)
 *  2. Push into the m_npcs list
 */
void MyGL::setupNPCs()
{
    // two fornite lamas on the jump training stadium
    // moving back & forth between two targets
    std::vector<glm::vec3> jump1To2 = {glm::vec3(33.f, 146.f, 33.f),
                                       glm::vec3(76.f, 152.f, 72.f)};
    std::vector<glm::vec3> jump2To1 = {glm::vec3(76.f, 152.f, 72.f),
                                       glm::vec3(33.f, 146.f, 33.f)};
    m_npcs.push_back(mkU<Lama>(this, glm::vec3(32.f, 148.f, 33.f),
                               m_terrain, m_player, GLAMA,
                               jump1To2,
                               glm::vec3(3.f, 0.f, 3.f),
                               2.f, 1.f,
                               7));
    m_npcs.push_back(mkU<Lama>(this, glm::vec3(73.f, 155.f, 73.f),
                               m_terrain, m_player, WLAMA,
                               jump2To1,
                               glm::vec3(3.f, 0.f, 3.f),
                               2.f, 1.f,
                               7));

    // one zombie dragon flying around the player
    m_npcs.push_back(mkU<ZombieDragon>(this, glm::vec3(65.f, 160.f, 32.f), m_terrain, m_player, ZDRAGON));

    // sheep on the grounds
    // moving around
    int nSheeps = 6;
    std::vector<glm::vec3> sheepGoals = {glm::vec3(-145, 137, -227),
                                         glm::vec3(-72, 148, -294),
                                         glm::vec3(0, 139, -48),
                                         glm::vec3(32, 138, 32)};

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rng = std::default_random_engine(seed);

    for (int i = 0; i < nSheeps; i++)
    {
        std::shuffle(sheepGoals.begin(), sheepGoals.end(), rng);
        m_npcs.push_back(mkU<Sheep>(this, glm::vec3(50.f + ((float) i) * 1.5f, 144.f, 32.f),
                                    m_terrain, m_player, SHEEP,
                                    sheepGoals,
                                    glm::vec3(1.f, 0.f, 1.f),
                                    2.f, 2.f,
                                    5));
    }


//    // Steve exploring the world
//    m_npcs.push_back(mkU<Steve>(this, glm::vec3(60.f, 145.f, 35.f),
//                                m_terrain, m_player, STEVE,
//                                std::vector<glm::vec3>(),
//                                glm::vec3(2.f, 0.f, 2.f),
//                                2.f, 2.f,
//                                25));
}
