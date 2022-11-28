#pragma once
#include <unordered_map>
#include "drawable.h"
#include "utils.h"
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QApplication>
#include <QFile>
#include <iostream>

#ifndef WIDGET_H
#define WIDGET_H

/**
 * This serves as widget's helper
 * to define various rectangular region in a single widget
 * for the operation inside each region or across regions
 * Often used with block in widget or selected frame
 **/
struct RecRegion
{
    glm::vec2 firstItemTopLeftUVCoord;
    glm::vec2 firstItemBottomRightUVCoord;
    glm::vec2 firstItemTopLeftScreenCoord;
    glm::vec2 firstItemBottomRightScreenCoord;
    glm::vec2 shiftDist;
    glm::ivec2 size;
    int capacity;
    RecRegion* prev;
    RecRegion* next;

    RecRegion() : firstItemTopLeftUVCoord(), firstItemBottomRightUVCoord(),
        firstItemTopLeftScreenCoord(), firstItemBottomRightScreenCoord(),
        shiftDist(), size(), prev(nullptr), next(nullptr)
    {}

    // without uv (for storing items)
    RecRegion(glm::vec2 topLeftScreen, glm::vec2 bottomRightScreen, glm::vec2 shiftDist, glm::vec2 size)
        : firstItemTopLeftScreenCoord(topLeftScreen), firstItemBottomRightScreenCoord(bottomRightScreen),
          shiftDist(shiftDist), size(size), prev(nullptr), next(nullptr)
    {
        capacity = size.x * size.y;
    }

    // with uv (for selected frame)
    RecRegion(glm::vec2 topLeftUV, glm::vec2 bottomRightUV, glm::vec2 topLeftScreen, glm::vec2 bottomRightScreen, glm::vec2 shiftDist, glm::vec2 size)
        : firstItemTopLeftUVCoord(topLeftUV), firstItemBottomRightUVCoord(bottomRightUV)

    {
        RecRegion(topLeftScreen, bottomRightScreen, shiftDist, size);
    }

};

class Widget : public Drawable
{
protected:
    // top-left and bottom-right uv coordinate of widget
    // 0 <= x, y <= 1 (origin at bottom-left)
    std::vector<glm::vec2> widgetUVCoord;

    // top-left and bottom-right screen coordinate of widget
    // -1 <= x, y <= 1 (origin at center)
    std::vector<glm::vec2> widgetScreenCoord;

    // stores info of all rectangular regions
    std::vector<uPtr<RecRegion>> regions;

    // information of all region info
    // In default, it should contain 6 information as follows (in order)
//     1. shift distance of the item in this region
//     2. size of the region
//     3. top-left screen coordinate of the first item in this region
//     4. bottom right screen coordinate of the first item in this region
//     5. top-left uv coordinate of the first item in this region
//     6. bottom right uv coordinate of the first item in this region
    std::vector<glm::vec2> regionInfo;

    // vector storing the info of all drawed item (for selected frame and block)
    // size: 8 for every item
    // 1. bottom-left, bottom-right, top-right and top-left of pos
    // 2. bottom-left, bottom-right, top-right and top-left of uv
    std::vector<std::vector<glm::vec2>> drawItems;

    // used for loading coordinates from text file
    std::unordered_map<std::string, std::pair<std::vector<glm::vec2>, int>> widgetInfoMap;

    void pushVec4ToBuffer(std::vector<float> &buf, const glm::vec4 &vec);
    void pushVec2ToBuffer(std::vector<float> &buf, const glm::vec2 &vec);

    // create new RecRegion object
    void createRegion(std::vector<glm::vec2> regionInfo);

public:
    Widget(OpenGLContext* context);
//    void setupWidgetCoord(float uv_x, float uv_y, float screen_x, float screen_y);
//    void setupSelectedFrameCoord(float uv_x, float uv_y, float screen_x, float screen_y);
//    void setupSelectedFrameShiftInfo(float dist_x, float dist_y, int colLimit, int rowLimit);
    void loadCoordFromText(const char* text_path);
    void insertNewInfos(std::string infoType, std::vector<glm::vec2> infos);

    // we draw the item with uv stored in RecRegion, usually for selected frame
    void addItem(int overallIdx);

    // read the uv and pos from specific region given overall index
    // for selected frame
    void storeItemIntoDrawVector(RecRegion* currRegion, int shiftX, int shiftY);

    virtual void createVBOdata();
};

#endif // WIDGET_H
