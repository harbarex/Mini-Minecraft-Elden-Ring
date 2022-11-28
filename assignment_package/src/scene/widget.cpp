#include "widget.h"

Widget::Widget(OpenGLContext *context)
    : Drawable(context)
{
    widgetInfoMap = {
        {"widgetUV", std::make_pair(widgetUVCoord, 2)},
        {"widgetScreen", std::make_pair(widgetScreenCoord, 2)},
        {"regionInfo", std::make_pair(regionInfo, 6)}
    };
}

/**
 * @brief pushVec4ToBuffer
 *  The helper func to push 4 elements into buffer array
 *  same as the code in chunk.cpp
 * @param buf
 * @param vec
 */
void Widget::pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec)
{
    for (int i = 0; i < 4; i++) {
        buf.push_back(vec[i]);
    }
}


/**
 * @brief pushVec2ToBuffer
 *   The helper func to push 2 elements into buffer array
 *   same as the code in chunk.cpp
 * @param buf
 * @param vec
 */
void Widget::pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec)
{
    for (int i = 0; i < 2; i++) {
        buf.push_back(vec[i]);
    }
}

void Widget::insertNewInfos(std::string infoType, std::vector<glm::vec2> infos) {
    for (int i = 0; i < (int)infos.size(); ++i) {
        widgetInfoMap[infoType].first.push_back(infos[i]);
    }

    if (infoType == "regionInfo") {
        createRegion(infos);
        widgetInfoMap[infoType].first.clear();
    }

    return;
}

void Widget::loadCoordFromText(const char* text_path) {
    // read text file
    QFile f(text_path);
    f.open(QIODevice::ReadOnly);
    QTextStream s(&f);
    QString line;
    bool isReadInfo = false;
    int dataReadCount = 0;
    int dataReadCountMax = 1;
    std::vector<glm::vec2> infos;
    std::string currInfoType;

    while (!s.atEnd()) {
        line = s.readLine();
        if (line.size() == 0 || line.startsWith("#")) {
            // skip empty line and line starts with #
            continue;
        }

        if (isReadInfo) {
            // store uv coordinate into temp array
            infos.push_back(glm::vec2(line.split(" ")[0].toDouble(), line.split(" ")[1].toDouble()));
            dataReadCount += 1;
        } else if (widgetInfoMap.find(line.toStdString()) != widgetInfoMap.end()) {
            // identify which block
            currInfoType = line.toStdString();
            dataReadCountMax = widgetInfoMap[line.toStdString()].second;
            isReadInfo = true;
        }

        if (dataReadCount == dataReadCountMax) {
            // store 2 uv coordinates into Widget object
            insertNewInfos(currInfoType, infos);
            infos.clear();
            dataReadCount = 0;
            isReadInfo = false;
        }
    }
}

void Widget::createRegion(std::vector<glm::vec2> regionInfo) {
    uPtr<RecRegion> region = mkU<RecRegion>();
    region->shiftDist = regionInfo[0];
    region->size = regionInfo[1];
    region->firstItemTopLeftScreenCoord = regionInfo[2];
    region->firstItemBottomRightScreenCoord = regionInfo[3];
    if (regionInfo.size() == 4) {
        return;
    }
    region->firstItemTopLeftUVCoord = regionInfo[4];
    region->firstItemBottomRightUVCoord = regionInfo[5];

    if (regions.size() > 0) {
        regions.back()->next = region.get();
        region->prev = regions.back().get();
    }

    regions.push_back(std::move(region));

}

/**
 * @brief Widget::addItem
 *  add draw item into widget object, for selected frame
 *  0-indexed, start from top-left corner of the first region
 * @param overallIdx : int, the index
 * @return
 */
void Widget::addItem(int overallIdx) {
    int remainingIdx = overallIdx;
    RecRegion* currRegion = regions.front().get();

    while (remainingIdx >= currRegion->capacity) {
        remainingIdx -= currRegion->capacity;
        currRegion = currRegion->next;
    }

    int shiftX = remainingIdx % currRegion->size.x;
    int shiftY = remainingIdx / currRegion->size.x;

    storeItemIntoDrawVector(currRegion, shiftX, shiftY);
}

void Widget::storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY) {
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

    glm::vec2 topLeftFrameUV = currRegion->firstItemTopLeftUVCoord;
    glm::vec2 bottomRightFrameUV = currRegion->firstItemBottomRightUVCoord;
    glm::vec2 topRightFrameUV(bottomRightFrameUV.x, topLeftFrameUV.y);
    glm::vec2 bottomLeftFrameUV(topLeftFrameUV.x, bottomRightFrameUV.y);

    drawItem.push_back(bottomLeftFrameUV);
    drawItem.push_back(bottomRightFrameUV);
    drawItem.push_back(topRightFrameUV);
    drawItem.push_back(topLeftFrameUV);

    drawItems.push_back(drawItem);
}

void Widget::createVBOdata(){

    int loadWidgetCount = 2;

    std::vector<GLuint> indices;
    std::vector<GLuint> faceIndices = {0, 1, 2, 0, 2, 3};
    std::vector<float> buffer_pos;
    std::vector<float> buffer_uv;
    int nVert = 0;

    for (int i = 0; i < loadWidgetCount; ++i) {
        // add indices for each face (4 vertices)
        for (int index : faceIndices) {
            indices.push_back(nVert + index);
        }
        // move the offset for indices
        nVert += 4;
    }

    // position of widget
    glm::vec2 topLeftPos = widgetInfoMap["widgetScreen"].first[0];
    glm::vec2 bottomRightPos = widgetInfoMap["widgetScreen"].first[1];
    glm::vec2 topRightPos(bottomRightPos.x, topLeftPos.y);
    glm::vec2 bottomLeftPos(topLeftPos.x, bottomRightPos.y);

    pushVec4ToBuffer(buffer_pos, glm::vec4(bottomLeftPos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(bottomRightPos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(topRightPos, 0.999999f, 1.f));
    pushVec4ToBuffer(buffer_pos, glm::vec4(topLeftPos, 0.999999f, 1.f));

    // uv of widget
    glm::vec2 topLeftUV = widgetInfoMap["widgetUV"].first[0];
    glm::vec2 bottomRightUV = widgetInfoMap["widgetUV"].first[1];
    glm::vec2 topRightUV(bottomRightUV.x, topLeftUV.y);
    glm::vec2 bottomLeftUV(topLeftUV.x, bottomRightUV.y);

    pushVec2ToBuffer(buffer_uv, bottomLeftUV);
    pushVec2ToBuffer(buffer_uv, bottomRightUV);
    pushVec2ToBuffer(buffer_uv, topRightUV);
    pushVec2ToBuffer(buffer_uv, topLeftUV);

    for (auto& drawItem : drawItems) {
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[0], 0.999999f, 1.f));
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[1], 0.999999f, 1.f));
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[2], 0.999999f, 1.f));
        pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[3], 0.999999f, 1.f));

        pushVec2ToBuffer(buffer_uv, drawItem[4]);
        pushVec2ToBuffer(buffer_uv, drawItem[5]);
        pushVec2ToBuffer(buffer_uv, drawItem[6]);
        pushVec2ToBuffer(buffer_uv, drawItem[7]);
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
