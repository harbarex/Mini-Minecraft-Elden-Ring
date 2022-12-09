#include "mygl.h"
#include "scene/npcs/sheep.h"
#include "scene/npcs/steve.h"
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
      m_progUnderwater(this), m_progLava(this), m_progNoOp(this), m_quad(this), m_progNPC(this),
      m_frameBuffer(this, this->width(), this->height(), this->devicePixelRatio()),
      m_terrain(this), m_player(glm::vec3(57.f, 155.f, 41.f), m_terrain), m_npcs(),
      frameCount(0),
      prevFrameTime(QDateTime::currentMSecsSinceEpoch()), textureAll(this), npcTextures(),
      prevExpandTime(QDateTime::currentMSecsSinceEpoch())
{

    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    // Tell the timer to redraw 60 times per second
    m_timer.start(16);
    setFocusPolicy(Qt::ClickFocus);
    m_inputs = InputBundle();
    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
    prevMouseX = width() / 2;
    prevMouseY = height() / 2;

    m_player.setBlocksHold();

    setupNPCs();
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    m_quad.destroyVBOdata();
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

    m_progNPC.create(":/glsl/lambert.vert.glsl", ":/glsl/npc.frag.glsl");

    m_quad.createVBOdata();

    createTexture();
    createNPCTextures();

    // create NPC's VBO and initialize NPC scene graph
    for (const uPtr<NPC> &npc : m_npcs)
    {
        npc->createVBOdata();
        npc->initSceneGraph();
    }

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
    glm::mat4 viewproj = m_player.mcr_camera.getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    m_progLambert.setViewProjMatrix(viewproj);
    m_progFlat.setViewProjMatrix(viewproj);
    m_progNPC.setViewProjMatrix(viewproj);

    m_progNoOp.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    m_progUnderwater.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));
    m_progLava.setDimensions(glm::ivec2(w * this->devicePixelRatio(), h * this->devicePixelRatio()));

    m_frameBuffer.resize(this->width(), this->height(), this->devicePixelRatio());
    m_frameBuffer.destroy();
    m_frameBuffer.create();

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

    // pass delta-time to Player::tick
    m_player.tick(deltaTime, m_inputs);

    // TODO: pass delta-time to NPC's tick as well
    if (frameCount > 15.f * 60.f)
    {
        std::cout << "tick npc"  << std::endl;
        for (const uPtr<NPC> &npc : m_npcs)
        {
            npc->tick(deltaTime);
        }
        std::cout << "finish npc tick" << std::endl;
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

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progNPC.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    m_progLambert.setTime(frameCount);
    m_progLava.setTime(frameCount);
    m_progUnderwater.setTime(frameCount);
    m_progNPC.setTime(frameCount);

    renderTerrain(TerrainDrawType::opaque);

    glDisable(GL_DEPTH_TEST);

    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    //m_progFlat.draw(m_worldAxes);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderTerrain(TerrainDrawType::transparent);
    // render NPCs
    renderNPCs();
    glDisable(GL_BLEND);



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

    frameCount++;
}

// TODO: Change this so it renders the nine zones of generated
// terrain that surround the player (refer to Terrain::m_generatedTerrain
// for more info)
void MyGL::renderTerrain(TerrainDrawType drawType) {

    // bind the texture
    bindTexture();

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
        m_inputs.debugButtonPressed = true;
    } else if (e->key() == Qt::Key_N) {
        m_inputs.nPressed = true;
        m_player.selectNextBlock(m_inputs);
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = true;
    } else if (e->key() == Qt::Key_F) {
        m_player.toggleFlightMode();
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
        m_inputs.debugButtonPressed = false;
    } else if (e->key() == Qt::Key_N) {
        m_inputs.nPressed = false;
    } else if (e->key() == Qt::Key_Space) {
        m_inputs.spacePressed = false;
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

    m_player.rotateCameraView(m_inputs);

    moveMouseToCenter();
}

void MyGL::mousePressEvent(QMouseEvent *e) {
    // TODO

    switch (e->button()) {
    case (Qt::LeftButton):
        m_inputs.leftMouseButtonPressed = true;
        break;
    case (Qt::RightButton):
        m_inputs.rightMouseButtonPressed = true;
        break;
    default:
        return;
    }
}

void MyGL::mouseReleaseEvent(QMouseEvent *e) {
    // TODO

    switch (e->button()) {
    case (Qt::LeftButton):
        m_inputs.leftMouseButtonPressed = false;
        break;
    case (Qt::RightButton):
        m_inputs.rightMouseButtonPressed = false;
        break;
    default:
        return;
    }
}

void MyGL::createTexture() {
    loadTextureUVCoord();
    textureAll.create(":/textures/minecraft_textures_all.png");
    textureAll.load(0);
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

// This function is used to load uv coordinate of the block from text file
void MyGL::loadTextureUVCoord() {

    // read text file
    QFile f(":/textures/uv_coord_texture_all.txt");
    f.open(QIODevice::ReadOnly);
    QTextStream s(&f);
    QString line;
    bool readCoord = false;
    int coordCount = 0;
    std::array<glm::vec2, 6> uvOffsets;
    BlockType currBlockType;

    while (!s.atEnd()) {
        line = s.readLine();
        if (line.size() == 0 || line.startsWith("#")) {
            // skip empty line and line starts with #
            continue;
        }

        if (readCoord) {
            // store uv coordinate into temp array
            uvOffsets[coordCount] = glm::vec2(line.split(" ")[0].toDouble(), line.split(" ")[1].toDouble());
            coordCount += 1;
        } else if (Block::blockTypeMap.find(line.toStdString()) != Block::blockTypeMap.end()) {
            // identify which block
            currBlockType = Block::blockTypeMap[line.toStdString()];
            readCoord = true;
        }

        if (coordCount == 6) {
            // store 6 uv coordinates into BlockCollection

            Block::insertNewUVCoord(currBlockType, uvOffsets);
            uvOffsets.empty();
            coordCount = 0;
            readCoord = false;
        }
    }
}

void MyGL::bindTexture() {
    textureAll.bind(0);
    m_progLambert.setTexture();
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


    // Steve exploring the world
    m_npcs.push_back(mkU<Steve>(this, glm::vec3(60.f, 145.f, 35.f),
                                m_terrain, m_player, STEVE,
                                std::vector<glm::vec3>(),
                                glm::vec3(2.f, 0.f, 2.f),
                                2.f, 2.f,
                                25));
}
