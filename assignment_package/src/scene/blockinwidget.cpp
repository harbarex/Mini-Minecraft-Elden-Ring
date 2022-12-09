#include "blockinwidget.h"

BlockInWidget::BlockInWidget(OpenGLContext *context)
    : Widget(context)
{
    setWidgetInfo();
}

BlockInWidget::~BlockInWidget() {}

/**
 * @brief BlockInWidget::addItem
 *  override addItem in Widget
 *  add block item into widget object (2D)
 * @param overallIdx : int, the index among all regions in the widget
 * @param uvCoords: array of 4 uv coordinates of current block face, (order: bottom-left, bottom-right, top-right, top-left)
 * @return
 */
void BlockInWidget::addItem(int overallIdx, std::array<glm::vec2, 4>& uvCoords) {
    RecRegion* currRegion = regions.front().get();
    int shiftX, shiftY;
    findRegionInfoFromIdx(overallIdx, currRegion, &shiftX, &shiftY);
    storeItemIntoDrawVector(currRegion, shiftX, shiftY, uvCoords);
}


/**
 * @brief BlockInWidget::addItem
 *  override addItem in Widget
 *  add block item into widget object (3D)
 * @param overallIdx : int, the index among all regions in the widget
 * @param uvCoords: array of 4 uv coordinates (order: bottom-left, bottom-right, top-right, top-left)
 * of 3 block face, (order: XPOS, YPOS, ZPOS
 * @return
 */
void BlockInWidget::addItem(int overallIdx, std::array<std::array<glm::vec2, 4>, 3>& uvCoords) {
    RecRegion* currRegion = regions.front().get();
    int shiftX, shiftY;
    findRegionInfoFromIdx(overallIdx, currRegion, &shiftX, &shiftY);
    storeItemIntoDrawVector(currRegion, shiftX, shiftY, uvCoords);
}

/**
 * @brief BlockInWidget::addItem
 *  override addItem in Widget
 *  add block item into widget object with given position and the length
 * @param pos : glm::vec2, the center position of the block, -1 <= pos.x, pos.y <= 1
 * @param len : glm::vec2, the width and height of the block, 0 <= len.x, len.y
 * @param uvCoords: array of 4 uv coordinates of current block face, (order: bottom-left, bottom-right, top-right, top-left)
 * @return
 */
void BlockInWidget::addItem(glm::vec2 pos, glm::vec2 len, std::array<glm::vec2, 4>& uvCoords) {
    std::vector<glm::vec2> drawItem;

    glm::vec2 topLeftFramePos = pos + glm::vec2(-len.x, len.y) / 2.f;
    glm::vec2 bottomRightFramePos = pos + glm::vec2(len.x, -len.y) / 2.f;
    glm::vec2 topRightFramePos(bottomRightFramePos.x, topLeftFramePos.y);
    glm::vec2 bottomLeftFramePos(topLeftFramePos.x, bottomRightFramePos.y);

    drawItem.push_back(bottomLeftFramePos);
    drawItem.push_back(bottomRightFramePos);
    drawItem.push_back(topRightFramePos);
    drawItem.push_back(topLeftFramePos);

    for (int i=0; i<4; ++i) {
        drawItem.push_back(uvCoords[i]);
    }

    drawItems.push_back(drawItem);
}

void BlockInWidget::addItem(glm::vec2 pos, glm::vec2 len, std::array<std::array<glm::vec2, 4>, 3>& uvCoords) {

    glm::vec2 topLeftFramePos = pos + glm::vec2(-len.x, len.y) / 2.f;
    glm::vec2 bottomRightFramePos = pos + glm::vec2(len.x, -len.y) / 2.f;
    glm::vec2 topRightFramePos(bottomRightFramePos.x, topLeftFramePos.y);
    glm::vec2 bottomLeftFramePos(topLeftFramePos.x, bottomRightFramePos.y);
    glm::vec2 centerFramePos = (topLeftFramePos + bottomRightFramePos + topRightFramePos + bottomLeftFramePos) / 4.f;

    // YPOS (top), XPOS (bottom-right), ZPOS (bottom-left)
    std::array<glm::vec2, 3> topLeftPos;
    std::array<glm::vec2, 3> bottomRightPos;
    std::array<glm::vec2, 3> topRightPos;
    std::array<glm::vec2, 3> bottomLeftPos;

    // convert to 3 faces
    // top face (YPOS)
    bottomLeftPos[1] = topLeftFramePos * 3.f/4.f + bottomLeftFramePos * 1.f/4.f;
    bottomRightPos[1] = centerFramePos;
    topRightPos[1] = topRightFramePos * 3.f/4.f + bottomRightFramePos * 1.f/4.f;
    topLeftPos[1] = (topLeftFramePos + topRightFramePos) / 2.f;

    // bottom right (XPOS)
    bottomLeftPos[0] = (bottomLeftFramePos + bottomRightFramePos) / 2.f;
    bottomRightPos[0] = topRightFramePos * 1.f/4.f + bottomRightFramePos * 3.f/4.f;
    topRightPos[0] = topRightFramePos * 3.f/4.f + bottomRightFramePos * 1.f/4.f;
    topLeftPos[0] = centerFramePos;

    // bottom left (ZPOS)
    bottomLeftPos[2] = topLeftFramePos * 1.f/4.f + bottomLeftFramePos * 3.f/4.f;
    bottomRightPos[2] = (bottomLeftFramePos + bottomRightFramePos) / 2.f;
    topRightPos[2] = centerFramePos;
    topLeftPos[2] = topLeftFramePos * 3.f/4.f + bottomLeftFramePos * 1.f/4.f;

    for (int i=0; i<3; ++i) {
        std::vector<glm::vec2> drawItem;
        drawItem.clear();
        drawItem.push_back(bottomLeftPos[i]);
        drawItem.push_back(bottomRightPos[i]);
        drawItem.push_back(topRightPos[i]);
        drawItem.push_back(topLeftPos[i]);

        for (int j=0; j<4; ++j) {
            drawItem.push_back(uvCoords[i][j]);
        }

        drawItems.push_back(drawItem);
    }

}

/**
 * @brief BlockInWidget::storeItemIntoDrawVector
 *  override storeItemIntoDrawVector in Widget
 *  helper function to store the item into drawItems for further vbo creation
 * @param currRegion : region where the item locates, here we use the pos and shift info
 * @param shiftX, shiftY : the coordinate (0-indexed) of the item in currRegion (unit: element, not pixel)
 * @param uvCoords: array of 4 uv coordinates of current block face, (order: bottom-left, bottom-right, top-right, top-left)
 */
void BlockInWidget::storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY, std::array<glm::vec2, 4>& uvCoords) {
    std::vector<glm::vec2> drawItem;
    glm::vec2 shift(shiftX * currRegion->shiftDist.x, -shiftY * currRegion->shiftDist.y);

    glm::vec2 topLeftFramePos = currRegion->firstItemTopLeftScreenCoord + shift;
    glm::vec2 bottomRightFramePos = currRegion->firstItemBottomRightScreenCoord + shift;
    glm::vec2 topRightFramePos(bottomRightFramePos.x, topLeftFramePos.y);
    glm::vec2 bottomLeftFramePos(topLeftFramePos.x, bottomRightFramePos.y);

    drawItem.push_back(bottomLeftFramePos);
    drawItem.push_back(bottomRightFramePos);
    drawItem.push_back(topRightFramePos);
    drawItem.push_back(topLeftFramePos);

    for (int i=0; i<4; ++i) {
        drawItem.push_back(uvCoords[i]);
    }

    drawItems.push_back(drawItem);
}

/**
 * @brief BlockInWidget::storeItemIntoDrawVector
 *  override storeItemIntoDrawVector in Widget (3D)
 *  helper function to store the item into drawItems for further vbo creation
 * @param currRegion : region where the item locates, here we use the pos and shift info
 * @param shiftX, shiftY : the coordinate (0-indexed) of the item in currRegion (unit: element, not pixel)
 * @param uvCoords: array of 4 uv coordinates of current block face, (order: bottom-left, bottom-right, top-right, top-left)
 */
void BlockInWidget::storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY, std::array<std::array<glm::vec2, 4>, 3>& uvCoords) {

    // get the overall position
    glm::vec2 shift(shiftX * currRegion->shiftDist.x, -shiftY * currRegion->shiftDist.y);

    glm::vec2 topLeftFramePos = currRegion->firstItemTopLeftScreenCoord + shift;
    glm::vec2 bottomRightFramePos = currRegion->firstItemBottomRightScreenCoord + shift;
    glm::vec2 topRightFramePos(bottomRightFramePos.x, topLeftFramePos.y);
    glm::vec2 bottomLeftFramePos(topLeftFramePos.x, bottomRightFramePos.y);
    glm::vec2 centerFramePos = (topLeftFramePos + bottomRightFramePos + topRightFramePos + bottomLeftFramePos) / 4.f;

    // YPOS (top), XPOS (bottom-right), ZPOS (bottom-left)
    std::array<glm::vec2, 3> topLeftPos;
    std::array<glm::vec2, 3> bottomRightPos;
    std::array<glm::vec2, 3> topRightPos;
    std::array<glm::vec2, 3> bottomLeftPos;

    // convert to 3 faces
    // top face (YPOS)
    bottomLeftPos[1] = topLeftFramePos * 3.f/4.f + bottomLeftFramePos * 1.f/4.f;
    bottomRightPos[1] = centerFramePos;
    topRightPos[1] = topRightFramePos * 3.f/4.f + bottomRightFramePos * 1.f/4.f;
    topLeftPos[1] = (topLeftFramePos + topRightFramePos) / 2.f;

    // bottom right (XPOS)
    bottomLeftPos[0] = (bottomLeftFramePos + bottomRightFramePos) / 2.f;
    bottomRightPos[0] = topRightFramePos * 1.f/4.f + bottomRightFramePos * 3.f/4.f;
    topRightPos[0] = topRightFramePos * 3.f/4.f + bottomRightFramePos * 1.f/4.f;
    topLeftPos[0] = centerFramePos;

    // bottom left (ZPOS)
    bottomLeftPos[2] = topLeftFramePos * 1.f/4.f + bottomLeftFramePos * 3.f/4.f;
    bottomRightPos[2] = (bottomLeftFramePos + bottomRightFramePos) / 2.f;
    topRightPos[2] = centerFramePos;
    topLeftPos[2] = topLeftFramePos * 3.f/4.f + bottomLeftFramePos * 1.f/4.f;

    for (int i=0; i<3; ++i) {
        std::vector<glm::vec2> drawItem;
        drawItem.clear();
        drawItem.push_back(bottomLeftPos[i]);
        drawItem.push_back(bottomRightPos[i]);
        drawItem.push_back(topRightPos[i]);
        drawItem.push_back(topLeftPos[i]);

        for (int j=0; j<4; ++j) {
            drawItem.push_back(uvCoords[i][j]);
        }

        drawItems.push_back(drawItem);
    }

}

void BlockInWidget::setWidgetInfo() {
    widgetInfoMap = {
        {"regionInfo", std::make_pair(regionInfo, 4)}
    };
}

/**
 * @brief Widget::createVBOdata
 *  inherited from Drawable
 *  for custom data that need to pass to GPU
 */
void BlockInWidget::createVBOdata() {

    std::vector<GLuint> indices;
    std::vector<GLuint> faceIndices = {0, 1, 2, 0, 2, 3};
    std::vector<float> buffer_pos;
    std::vector<float> buffer_uv;
    int nVert = 0;

    for (auto& drawItem : drawItems) {
        for (int i=0; i<4; ++i) {
           pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[i], 0.999999f, 1.f));
           pushVec2ToBuffer(buffer_uv, drawItem[i+4]);
        }

        // add indices for each face (4 vertices)
        for (int index : faceIndices) {
            indices.push_back(nVert + index);
        }
        // move the offset for indices
        nVert += 4;
    }

    m_count = indices.size();

    generateIdx();
    bindIdx();
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    generatePos();
    bindPos();
    mp_context->glBufferData(GL_ARRAY_BUFFER, buffer_pos.size() * sizeof(float), buffer_pos.data(), GL_STATIC_DRAW);

    generateUV();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    mp_context->glBufferData(GL_ARRAY_BUFFER, buffer_uv.size() * sizeof(float), buffer_uv.data(), GL_STATIC_DRAW);

    drawItems.clear();
}
