#ifndef MYGL_H
#define MYGL_H

#include "framebuffer.h"
#include "openglcontext.h"
#include "scene/quad.h"
#include "scene/worldaxes.h"
#include "scene/camera.h"
#include "scene/terrain.h"
#include "scene/player.h"
#include "scene/block.h"
#include "scene/npc.h"
#include "scene/widget.h"
#include "scene/blockinwidget.h"
#include "scene/text.h"
#include "texture.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QApplication>
#include <smartpointerhelp.h>


class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    WorldAxes m_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    ShaderProgram m_progLambert;// A shader program that uses lambertian reflection
    ShaderProgram m_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)

    // Post-process Shaders
    ShaderProgram m_progUnderwater;
    ShaderProgram m_progLava;
    ShaderProgram m_progNoOp;
    ShaderProgram m_progInventoryWidgetOnHand;
    ShaderProgram m_progInventoryItemOnHand;
    ShaderProgram m_progInventoryWidgetInContainer;
    ShaderProgram m_progInventoryItemInContainer;
    ShaderProgram m_progGrabbedItem;
    ShaderProgram m_progText;
    Quad m_quad;
    Widget* inventoryWidgetOnHand;
    Widget* inventoryWidgetInContainer;
    BlockInWidget* inventoryItemsOnHand;
    BlockInWidget* inventoryItemsInContainer;
    BlockInWidget* grabbedItem;
    std::vector<uPtr<Widget>> widgets;
    uPtr<Text> textOnScreen;

    // NPC
    ShaderProgram m_progNPC;

    FrameBuffer m_frameBuffer;

    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry to o much about this. Just know it is necessary in order to render geometry.

    Terrain m_terrain; // All of the Chunks that currently comprise the world.
    Player m_player; // The entity controlled by the user. Contains a camera to display what it sees as well.
    InputBundle m_inputs; // A collection of variables to be updated in keyPressEvent, mouseMoveEvent, mousePressEvent, etc.

    std::vector<uPtr<NPC>> m_npcs; // A collection of npcs

    QTimer m_timer; // Timer linked to tick(). Fires approximately 60 times per second.

    int frameCount; // the number of processing frame, treated as time in shader
    long long prevFrameTime; // time in previous frame, for calculating delta-time in player tick funciton

    int prevMouseX;
    int prevMouseY;

    bool mouseCursorMode; // Mouse cursor can move or not

    Texture textureAll;
    Texture inventoryWidgetOnHandTexture;
    Texture inventoryWidgetInContainerTexture;
    Texture textureFont;

    std::unordered_map<NPCTexture, Texture> npcTextures;

    void moveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.

    void sendPlayerDataToGUI() const;


    void createNPCTextures();
    void loadNPCTextureUVCoord();

    void createTexture(Texture& texture, const char* img_path, int slot);
    void initWidget();
    void initText();
    void bindTexture(Texture& texture, ShaderProgram& shaderProgram, int slot);

    void toggleMouseCursorMode();

    void setupNPCs();
    void renderNPCs();


    long long prevExpandTime;

    glm::vec2 convertPosToNormalizedPos(glm::vec2 pixelPos);
    glm::vec2 convertPosToNormalizedPos(QMouseEvent *e);

    BlockType grabbedItemType;
    std::array<std::array<glm::vec2, 4>, 3> grabbedItemUVCoords;
    void drawGrabbedItem();


public:
    explicit MyGL(QWidget *parent = nullptr);
    ~MyGL();

    // Called once when MyGL is initialized.
    // Once this is called, all OpenGL function
    // invocations are valid (before this, they
    // will cause segfaults)
    void initializeGL() override;
    // Called whenever MyGL is resized.
    void resizeGL(int w, int h) override;
    // Called whenever MyGL::update() is called.
    // In the base code, update() is called from tick().
    void paintGL() override;

    // Called from paintGL().
    // Calls Terrain::draw().
    void renderTerrain(TerrainDrawType drawType);

    // Called from paintGL()
    // Render the shader program with simple texture map
    void renderTexture(Texture& texture, ShaderProgram& shaderProgram, int slot, Drawable* d);



protected:
    // Automatically invoked when the user
    // presses a key on the keyboard
    void keyPressEvent(QKeyEvent *e);
    // Automatically invoked when the user
    // releases a key on the keyboard
    void keyReleaseEvent(QKeyEvent *e);
    // Automatically invoked when the user
    // moves the mouse
    void mouseMoveEvent(QMouseEvent *e);
    // Automatically invoked when the user
    // presses a mouse button
    void mousePressEvent(QMouseEvent *e);
    // Automatically invoked when the user
    // releases a mouse button
    void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void tick(); // Slot that gets called ~60 times per second by m_timer firing.

signals:
    void sig_sendPlayerPos(QString) const;
    void sig_sendPlayerVel(QString) const;
    void sig_sendPlayerAcc(QString) const;
    void sig_sendPlayerLook(QString) const;
    void sig_sendPlayerChunk(QString) const;
    void sig_sendPlayerTerrainZone(QString) const;
};


#endif // MYGL_H
