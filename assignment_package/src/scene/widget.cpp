#include "widget.h"

Widget::Widget(OpenGLContext *context)
    : Drawable(context)
{
    setWidgetInfo();
}

Widget::~Widget() {}

/**
 * @brief Widget::insertNewInfos
 *   helper function to store each info in text file into widgetInfoMap
 * @param infoType : string, key in widgetInfoMap
 * @param infos : vector of glm::vec2, data associated with the info
 */
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

/**
 * @brief Widget::loadCoordFromText
 *   Load coordinate information from given text file
 *   need to define the variable and the size (how manay rows) in widgetInfoMap
 * @param text_path : path to text file (also need to add path to qrc file)
 */
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

/**
 * @brief Widget::createRegion
 *   helper function to pass regionInfo data into RecRegion struct (regions vector)
 * @param regionInfo : vector of glm::vec2, data associated with regionInfo
 */
void Widget::createRegion(std::vector<glm::vec2> regionInfo) {
    uPtr<RecRegion> region = mkU<RecRegion>();
    region->shiftDist = regionInfo[0];
    region->size = regionInfo[1];
    region->firstItemTopLeftScreenCoord = regionInfo[2];
    region->firstItemBottomRightScreenCoord = regionInfo[3];
    if (regionInfo.size() == 6) {
        region->firstItemTopLeftUVCoord = regionInfo[4];
        region->firstItemBottomRightUVCoord = regionInfo[5];
    }

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
    RecRegion* currRegion = regions.front().get();
    int shiftX, shiftY;
    findRegionInfoFromIdx(overallIdx, currRegion, &shiftX, &shiftY);
    storeItemIntoDrawVector(currRegion, shiftX, shiftY);
}

/**
 * @brief Widget::findRegionInfoFromIdx
 *  add draw item into widget object, for selected frame
 *  0-indexed, start from top-left corner of the first region
 * @param overallIdx : int, the index of item among all regions in current widget
 * @return currRegion : the region where the item located
 * @return shiftX, shiftY : the coordinate (0-indexed) of the item in currRegion (unit: element, not pixel)
 */
void Widget::findRegionInfoFromIdx(int overallIdx, RecRegion* currRegion, int* shiftX, int* shiftY) {
    int remainingIdx = overallIdx;
    while (remainingIdx >= currRegion->capacity) {
        remainingIdx -= currRegion->capacity;
        currRegion = currRegion->next;
    }

    *shiftX = remainingIdx % currRegion->size.x;
    *shiftY = remainingIdx / currRegion->size.x;
}

/**
 * @brief Widget::getWidgetItemNumberInfo
 *  get the number information based on the region where current index (item) located
 * @param overallIdx : int, the index of item among all regions in current widget
 * @return top_left_pos : the top-left position where the number of first item located (-1 < pos.x, pos.y < 1)
 * @return height : the height of the text (0 < height < 2)
 */
void Widget::getWidgetItemNumberInfo(int overallIdx, glm::vec2* top_left_pos, float* height) {
    RecRegion* currRegion = regions.front().get();
    int shiftX, shiftY;
    findRegionInfoFromIdx(overallIdx, currRegion, &shiftX, &shiftY);

    // hard-code top-left number position
    glm::vec2 pos;
    pos.x = 0.75f * currRegion->firstItemTopLeftScreenCoord.x + 0.25f * currRegion->firstItemBottomRightScreenCoord.x;
    pos.y = 0.5f * currRegion->firstItemTopLeftScreenCoord.y + 0.5f * currRegion->firstItemBottomRightScreenCoord.y;
    *top_left_pos = pos + glm::vec2(shiftX * currRegion->shiftDist.x, -shiftY * currRegion->shiftDist.y);
    // hard-code number height
    *height = 0.5 * 0.85 * (currRegion->firstItemTopLeftScreenCoord.y - currRegion->firstItemBottomRightScreenCoord.y);
}


/**
 * @brief Widget::storeItemIntoDrawVector
 *  helper function to store the item into drawItems for further vbo creation
 * @param currRegion : region where the item locates, here we use the uv, pos and shift info
 * @param shiftX, shiftY : the coordinate (0-indexed) of the item in currRegion (unit: element, not pixel)
 */
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

void Widget::setWidgetInfo() {
    widgetInfoMap = {
        {"widgetUV", std::make_pair(widgetUVCoord, 2)},
        {"widgetScreen", std::make_pair(widgetScreenCoord, 2)},
        {"regionInfo", std::make_pair(regionInfo, 6)}
    };
}

/**
 * @brief Widget::createVBOdata
 *  inherited from Drawable
 *  store the data into vbo
 */
void Widget::createVBOdata(){

    int loadWidgetCount = 1 + drawItems.size();

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
        for (int i=0; i<4; ++i) {
           pushVec4ToBuffer(buffer_pos, glm::vec4(drawItem[i], 0.999999f, 1.f));
           pushVec2ToBuffer(buffer_uv, drawItem[i+4]);
        }
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
