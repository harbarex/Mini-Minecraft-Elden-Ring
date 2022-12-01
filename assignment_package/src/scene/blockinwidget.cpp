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
 *  add block item into widget object
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
 * @brief BlockInWidget::storeItemIntoDrawVector
 *  override storeItemIntoDrawVector in Widget
 *  helper function to store the item into drawItems for further vbo creation
 * @param currRegion : region where the item locates, here we use the pos and shift info
 * @param shiftX, shiftY : the coordinate (0-indexed) of the item in currRegion (unit: element, not pixel)
 * @param uvCoords: array of 4 uv coordinates of current block face, (order: bottom-left, bottom-right, top-right, top-left)
 */
void BlockInWidget::storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY, std::array<glm::vec2, 4>& uvCoords) {
    std::vector<glm::vec2> drawItem;
    glm::vec2 shift(shiftX * currRegion->shiftDist.x, shiftY * currRegion->shiftDist.y);

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
