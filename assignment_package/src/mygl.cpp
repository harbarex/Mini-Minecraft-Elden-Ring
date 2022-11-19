#include "mygl.h"
#include <glm_includes.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include <QFile>

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      m_worldAxes(this),
      m_progLambert(this), m_progFlat(this),
      m_terrain(this), m_player(glm::vec3(48.f, 200.f, 48.f), m_terrain),
      prevFrameTime(QDateTime::currentMSecsSinceEpoch()), textureAll(this),
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
}

MyGL::~MyGL() {
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
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

    //Create the instance of the world axes
    m_worldAxes.createVBOdata();

    // Create and set up the diffuse shader
    m_progLambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    m_progFlat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
//    m_progInstanced.create(":/glsl/instanced.vert.glsl", ":/glsl/lambert.frag.glsl");

    createTexture();

    // Set a color with which to draw geometry.
    // This will ultimately not be used when you change
    // your program to render Chunks with vertex colors
    // and UV coordinates
    m_progLambert.setGeometryColor(glm::vec4(0,1,0,1));

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
    else if ((QDateTime::currentMSecsSinceEpoch() - prevExpandTime) >= 300)
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

    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progLambert.setViewProjMatrix(m_player.mcr_camera.getViewProj());
//    m_progInstanced.setViewProjMatrix(m_player.mcr_camera.getViewProj());

    renderTerrain(TerrainDrawType::opaque);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    renderTerrain(TerrainDrawType::transparent);
    glDisable(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    m_progFlat.setModelMatrix(glm::mat4());
    m_progFlat.setViewProjMatrix(m_player.mcr_camera.getViewProj());
    m_progFlat.draw(m_worldAxes);
    glEnable(GL_DEPTH_TEST);
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
